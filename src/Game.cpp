#include "Game.h"

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>

#include "Player.h"
#include "TileMap.h"
#include "crate_img.h"
#include "physics/PhysicsWorld.h"
#include "platform_utils.h"
#include "player_img.h"
#include "utils.h"

namespace {

// Tile numbers in assets/background.png (10 tiles per row, 32 px each)
constexpr int kSandTile = 50;  // 50 and 51 are sand variants
constexpr int kIceTile = 52;   // 52 and 53 are ice variants

sf::Color darker(sf::Color c) { return sf::Color(c.r / 2, c.g / 2, c.b / 2); }
sf::Color lighter(sf::Color c) {
  return sf::Color(std::min(255, c.r + 60), std::min(255, c.g + 60),
                   std::min(255, c.b + 60));
}

// Simple pixel-art disc: outline, flat fill and a highlight on the top left
std::unique_ptr<sf::Texture> makeDiscTexture(unsigned int radius,
                                             sf::Color fill) {
  const unsigned int d = radius * 2;
  sf::Image image({d, d}, sf::Color::Transparent);
  const float c = radius - 0.5f;
  const float r_outer = radius * radius;
  const float r_inner = (radius - 1.5f) * (radius - 1.5f);
  for (unsigned int y = 0; y < d; ++y)
    for (unsigned int x = 0; x < d; ++x) {
      const float dx = x - c, dy = y - c;
      const float dist = dx * dx + dy * dy;
      if (dist > r_outer) continue;
      sf::Color color = dist > r_inner             ? darker(fill)
                        : dx + dy < -0.6f * radius ? lighter(fill)
                                                   : fill;
      image.setPixel({x, y}, color);
    }
  auto texture = std::make_unique<sf::Texture>();
  if (!texture->loadFromImage(image)) printf("Creating disc texture failed\n");
  return texture;
}

// Simple pixel-art box: outline plus a lighter top and left edge
std::unique_ptr<sf::Texture> makeBoxTexture(sf::Vector2u size, sf::Color fill) {
  sf::Image image(size, fill);
  for (unsigned int y = 0; y < size.y; ++y)
    for (unsigned int x = 0; x < size.x; ++x) {
      if (x == 0 || y == 0 || x == size.x - 1 || y == size.y - 1)
        image.setPixel({x, y}, darker(fill));
      else if (x == 1 || y == 1)
        image.setPixel({x, y}, lighter(fill));
    }
  auto texture = std::make_unique<sf::Texture>();
  if (!texture->loadFromImage(image)) printf("Creating box texture failed\n");
  return texture;
}

}  // namespace

int runGame(int framerate = 60) {
  const float VIEW_SCALE = 0.25f;
  const unsigned int SMALL_FONT_SIZE = 32;
  const unsigned int OVERLAY_FONT_SIZE = 20;
  // Physics runs on a fixed timestep (in seconds) independent of the frame
  // rate; long stalls are clamped so the simulation never tries to catch up
  // on more than this much time at once.
  const float PHYSICS_DT = 1.f / 120.f;
  const float MAX_FRAME_TIME = 0.25f;
  float physics_accumulator = 0.f;
  GameState game_state = GameState::Menu;
  int* m_p;  // Pointer to dynamically allocated array for tile map data
  sf::Clock m_clock;
  // Up to 4 drawing layers
  std::vector<sf::Sprite*> m_sprite_layer[4];
  std::vector<Object*> m_object_list;
  physics::PhysicsWorld m_world;  // top-down game: no gravity
  uint8_t m_input = 0;
  const std::string resourcePath = getResourcePath();
  sf::RenderWindow m_window(sf::VideoMode({1000, 800}), "Phantom Force");
  m_window.setFramerateLimit(framerate);

  sf::Texture tex;
  sf::Texture crate_tex;
  sf::Font font;

  // Speeds in pixels per second, acceleration in pixels / s^2, damping in 1/s
  MoveStats def({300.0f, 1800.0f, 15.0f, 1.0f});

  // Viewport of 250 x 200, 1/4 of the original window 1000 x 800
  sf::View view = m_window.getView();
  sf::Vector2f minCenter = sf::Vector2f(125.f, 100.f);
  view.zoom(VIEW_SCALE);
  m_window.setView(view);

  if (!(tex.loadFromMemory(player_img, player_img_len))) {
    printf("Loading texture failed\n");
  }
  // The crate drawing is only 14x14 inside a 32x32 image: crop the texture
  // to the visible crate so its collider fits, and repeat it for crate walls.
  sf::Image crate_image;
  if (!crate_image.loadFromMemory(crate_img, crate_img_len) ||
      !crate_tex.loadFromImage(crate_image, false, opaqueBounds(crate_image))) {
    printf("Loading texture failed\n");
  }
  crate_tex.setRepeated(true);
  const sf::Vector2f crate_size(crate_tex.getSize());
  // Textures generated for the physics demo objects
  std::vector<std::unique_ptr<sf::Texture>> generated_textures;
  if (!font.openFromFile(resourcePath + "EBB.ttf")) {
    printf("Loading font failed\n");
  }
  const_cast<sf::Texture&>(font.getTexture(SMALL_FONT_SIZE)).setSmooth(false);
  const_cast<sf::Texture&>(font.getTexture(OVERLAY_FONT_SIZE)).setSmooth(false);

  Player* play = new Player(tex, &def, 6.f);
  play->setPosition(sf::Vector2f(10, 10));
  m_sprite_layer[1].push_back(play);
  m_world.addBody(play);

  // --- Physics demo objects --------------------------------------------------
  // Densities (mass per pixel^2) so bigger objects of a material are heavier
  const float LIGHT = 0.001f, MEDIUM = 0.003f, HEAVY = 0.01f;
  auto addBody = [&](auto* obj, sf::Vector2f pos, float density) {
    obj->setPosition(pos);
    obj->setDensity(density);
    m_object_list.push_back(obj);
    m_sprite_layer[2].push_back(obj);
    m_world.addBody(obj);
    return obj;
  };
  auto disc = [&](unsigned int radius, sf::Color color) -> sf::Texture& {
    generated_textures.push_back(makeDiscTexture(radius, color));
    return *generated_textures.back();
  };
  auto boxTex = [&](sf::Vector2f size, sf::Color color) -> sf::Texture& {
    generated_textures.push_back(makeBoxTexture(sf::Vector2u(size), color));
    return *generated_textures.back();
  };

  // A medium ball the player can shove around; the floor's friction slows it
  Circle* ball = addBody(new Circle(disc(20, sf::Color(90, 140, 200)), 20.f),
                         {160.f, 40.f}, MEDIUM);
  ball->setRestitution(0.6f);

  // A wall built from crates (mass 0 => static)
  addBody(new Rectangle(crate_tex, crate_size.componentWiseMul({3.f, 28.f})),
          {90.f, 240.f}, 0.f);

  // Loose crates that slide, spin and pile up against the wall (which spans
  // x = 69..111, so they all start just clear of it)
  for (const sf::Vector2f& pos :
       {sf::Vector2f(50.f, 90.f), sf::Vector2f(50.f, 125.f),
        sf::Vector2f(16.f, 108.f)}) {
    Rectangle* crate = addBody(new Rectangle(crate_tex), pos, MEDIUM);
    crate->setFriction(0.4f);
  }

  // Light, bouncy balls on the ice patch
  for (const sf::Vector2f& pos :
       {sf::Vector2f(190.f, 100.f), sf::Vector2f(230.f, 120.f),
        sf::Vector2f(270.f, 90.f)}) {
    Circle* marble =
        addBody(new Circle(disc(4, sf::Color(120, 220, 140)), 4.f), pos, LIGHT);
    marble->setRestitution(0.9f);
    marble->setFriction(0.1f);
  }

  // A heavy boulder that barely budges and a heavy steel block
  Circle* boulder =
      addBody(new Circle(disc(16, sf::Color(110, 105, 100)), 16.f),
              {200.f, 240.f}, HEAVY);
  boulder->setRestitution(0.1f);
  boulder->setFriction(0.7f);
  Rectangle* block =
      addBody(new Rectangle(boxTex({30.f, 30.f}, sf::Color(130, 140, 150)),
                            {30.f, 30.f}),
              {250.f, 300.f}, HEAVY);
  block->setRestitution(0.2f);
  block->setFriction(0.5f);

  // A long plank that spins easily when hit off centre
  Rectangle* plank = addBody(
      new Rectangle(boxTex({60.f, 8.f}, sf::Color(170, 120, 70)), {60.f, 8.f}),
      {150.f, 300.f}, MEDIUM);
  plank->setFriction(0.4f);

  // Sand patch: a few mid-size balls that stop quickly
  for (const sf::Vector2f& pos :
       {sf::Vector2f(30.f, 220.f), sf::Vector2f(60.f, 260.f)}) {
    Circle* ball2 =
        addBody(new Circle(disc(8, sf::Color(200, 90, 80)), 8.f), pos, MEDIUM);
    ball2->setRestitution(0.4f);
  }

  // Floor surfaces come from the background tiles under each body
  auto surfaceForTile = [](int tile) {
    if (tile == kSandTile || tile == kSandTile + 1)
      return physics::Surface::sand();
    if (tile == kIceTile || tile == kIceTile + 1)
      return physics::Surface::ice();
    return physics::Surface::metal();
  };

  TileMap background_map;
  m_p = new int[10000];
  for (int i = 0; i < 10000; i++) m_p[i] = i % 4;
  m_p[101] = 15;
  m_p[102] = 16;
  m_p[203] = 15;
  m_p[302] = 15;
  // Terrain patches: an ice rink to the right of the crate wall and a sandy
  // corner below the crates (tile coordinates, 32 px per tile)
  auto paintTiles = [&](int x0, int y0, int x1, int y1, int first_tile) {
    for (int y = y0; y <= y1; y++)
      for (int x = x0; x <= x1; x++)
        m_p[x + y * 100] = first_tile + (x * 7 + y * 13) % 2;
  };
  paintTiles(5, 2, 9, 5, kIceTile);
  paintTiles(0, 6, 2, 9, kSandTile);
  if (!background_map.loadTileset(resourcePath + "background.png",
                                  sf::Vector2u(32, 32)))
    printf("Loading tileset failed\n");
  background_map.loadMap(m_p, 100, 100, view);
  background_map.flash(sf::Vector2i(0, 0));
  m_world.setDefaultSurface(physics::Surface::metal());
  m_world.setSurfaceSampler([&](sf::Vector2f p) {
    return surfaceForTile(background_map.getTile(p));
  });

  // Create a transparent overlay for the pause effect
  sf::RectangleShape pauseOverlay(sf::Vector2f(m_window.getSize()));
  pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150));  // Black with alpha

  // Define menu
  sf::Text titleText(font);
  titleText.setString("Phantom Force");
  titleText.setCharacterSize(48);
  sf::FloatRect titleBounds = titleText.getLocalBounds();
  titleText.setOrigin(sf::Vector2f(titleBounds.position.x + titleBounds.size.x / 2.0f,
                                   titleBounds.position.y + titleBounds.size.y / 2.0f));
  titleText.setPosition(sf::Vector2f(m_window.getSize().x / 2.0f,
                                     m_window.getSize().y / 3.0f));

  Button startButton(
      sf::Vector2f(200, 50),
      sf::Vector2f((std::floor(m_window.getSize().x - 200) / 2.0f),
                   std::floor((m_window.getSize().y - 50) / 2.0f) + 50),
      "Start Game", font, SMALL_FONT_SIZE);

  Button quitButton(
      sf::Vector2f(200, 50),
      sf::Vector2f((std::floor(m_window.getSize().x - 200) / 2.0f),
                   std::floor((m_window.getSize().y - 50) / 2.0f) + 120),
      "Quit", font, SMALL_FONT_SIZE);

  auto drawMenu = [&]() {
    m_window.setView(m_window.getDefaultView());
    m_window.draw(titleText);
    m_window.draw(startButton);
    m_window.draw(quitButton);
  };

  bool using_controller = false;
  unsigned int active_joystick_id = 0;
  float last_play_dir = 0.0f;

  // Frame statistics for the debug overlay
  unsigned int draw_calls = 0;
  unsigned int physics_steps_frame = 0;
  float physics_us_frame = 0.f;
  float fps_smoothed = 0.f;
  sf::Text stats_text(font, "", OVERLAY_FONT_SIZE);
  stats_text.setFillColor(sf::Color::White);
  stats_text.setOutlineColor(sf::Color::Black);
  stats_text.setOutlineThickness(2.f);
  stats_text.setPosition(sf::Vector2f(10.f, 10.f));
  auto draw = [&](const sf::Drawable& drawable) {
    m_window.draw(drawable);
    ++draw_calls;
  };

  // Define a lambda to draw the game world
  auto drawWorld = [&]() {
    m_window.setView(view);  // Use camera view
    background_map.loadVertexChunk(view.getCenter());
    draw(background_map);

    // Draw layers
    for (int i = 3; i >= 1; i--) {
      for (auto* sprite : m_sprite_layer[i]) {
        draw(*sprite);
      }
    }

    // Collision shapes and contact normals on top of the sprites
    if (Object::g_draw_collisions) draw_calls += m_world.drawDebug(m_window);
  };

  // Performance read-out shown with the collision overlay (Space)
  auto drawStats = [&](float dt) {
    const physics::Stats& stats = m_world.getStats();
    const float physics_per_step =
        physics_steps_frame ? physics_us_frame / physics_steps_frame : 0.f;
    char text[512];
    std::snprintf(
        text, sizeof(text),
        "FPS %5.1f  frame %5.2f ms\n"
        "physics %6.1f us/frame  (%u steps, %5.1f us/step)\n"
        "bodies %zu  pairs %zu  tests %zu  contacts %zu  resting %zu\n"
        "draw calls %u",
        fps_smoothed, dt * 1000.f, physics_us_frame, physics_steps_frame,
        physics_per_step, stats.bodies, stats.broadphase_pairs,
        stats.narrowphase_tests, stats.contacts, stats.resting_pairs,
        draw_calls);
    stats_text.setString(text);
    m_window.setView(m_window.getDefaultView());
    m_window.draw(stats_text);
  };

  while (m_window.isOpen()) {
    const float dt = m_clock.restart().asSeconds();
    if (dt > 0.f) fps_smoothed = converge(1.f / dt, fps_smoothed, 0.05f);
    draw_calls = 0;

    // Global input events
    while (const std::optional event = m_window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        m_window.close();
      }
      if (const auto* joyMoved = event->getIf<sf::Event::JoystickMoved>()) {
        if (std::abs(joyMoved->position) > 15.0f) {
          using_controller = true;
          active_joystick_id = joyMoved->joystickId;
        }
      }
      if (const auto* joyPressed = event->getIf<sf::Event::JoystickButtonPressed>()) {
        using_controller = true;
        active_joystick_id = joyPressed->joystickId;
      }
      if (event->is<sf::Event::MouseMoved>() || event->is<sf::Event::MouseMovedRaw>() || event->is<sf::Event::KeyPressed>() || event->is<sf::Event::MouseButtonPressed>()) {
        using_controller = false;
      }

      // Handle input based on specific states if needed
      if (game_state == GameState::Playing || game_state == GameState::Paused)
          [[likely]] {
        if (const sf::Event::KeyPressed* keyPressed =
                event->getIf<sf::Event::KeyPressed>()) {
          switch (keyPressed->scancode) {
            case sf::Keyboard::Scancode::A:
              m_input |= Player::kLeft;
              break;
            case sf::Keyboard::Scancode::D:
              m_input |= Player::kRight;
              break;
            case sf::Keyboard::Scancode::W:
              m_input |= Player::kUp;
              break;
            case sf::Keyboard::Scancode::S:
              m_input |= Player::kDown;
              break;
            case sf::Keyboard::Scancode::Space:
              Object::g_draw_collisions = !Object::g_draw_collisions;
              break;
            default:
              break;
          }
        } else if (const sf::Event::KeyReleased* keyReleased =
                       event->getIf<sf::Event::KeyReleased>()) {
          switch (keyReleased->scancode) {
            case sf::Keyboard::Scancode::A:
              m_input &= ~Player::kLeft;
              break;
            case sf::Keyboard::Scancode::D:
              m_input &= ~Player::kRight;
              break;
            case sf::Keyboard::Scancode::W:
              m_input &= ~Player::kUp;
              break;
            case sf::Keyboard::Scancode::S:
              m_input &= ~Player::kDown;
              break;
            [[unlikely]] case sf::Keyboard::Scancode::Escape:
              if (game_state == GameState::Paused)
                game_state = GameState::Playing;
              else if (game_state == GameState::Playing)
                game_state = GameState::Paused;
              break;
            default:
              break;
          }
        }
      }

      // Menu specific inputs
      if (game_state == GameState::Menu) [[unlikely]] {
        // Handle menu clicks
        if (const sf::Event::MouseButtonPressed* mousePressed =
                event->getIf<sf::Event::MouseButtonPressed>()) {
          if (mousePressed->button == sf::Mouse::Button::Left) {
            if (startButton.isMouseOver(m_window)) {
              game_state = GameState::Playing;
            } else if (quitButton.isMouseOver(m_window)) {
              m_window.close();
            }
          }
        }
      }

      // Handle Controller specific events
      if (const sf::Event::JoystickButtonPressed* joystickPressed =
              event->getIf<sf::Event::JoystickButtonPressed>()) {
        const unsigned int BTN_A = 0;
        const unsigned int BTN_START = 7;

        if (game_state == GameState::Menu && joystickPressed->button == BTN_A) {
          game_state = GameState::Playing;
        } else if (joystickPressed->button == BTN_START) {
          if (game_state == GameState::Paused) {
            game_state = GameState::Playing;
          } else if (game_state == GameState::Playing) {
            game_state = GameState::Paused;
          }
        }
      }
    }

    // Update logic
    if (game_state == GameState::Playing) {
      // Handle Controller Movement Input
      uint8_t current_input = m_input;
      if (using_controller) {
        float x = sf::Joystick::getAxisPosition(active_joystick_id, sf::Joystick::Axis::X);
        float y = sf::Joystick::getAxisPosition(active_joystick_id, sf::Joystick::Axis::Y);
        float povX = sf::Joystick::getAxisPosition(active_joystick_id, sf::Joystick::Axis::PovX);
        float povY = sf::Joystick::getAxisPosition(active_joystick_id, sf::Joystick::Axis::PovY);
        const float deadzone = 15.0f;

        if (x > deadzone || povX > deadzone) current_input |= Player::kRight;
        if (x < -deadzone || povX < -deadzone) current_input |= Player::kLeft;
        if (y > deadzone || povY < -deadzone) current_input |= Player::kDown; // SFML Joystick Y is down-positive, PovY is up-positive
        if (y < -deadzone || povY > deadzone) current_input |= Player::kUp;
      }

      // Step the simulation in fixed increments
      physics_accumulator += std::min(dt, MAX_FRAME_TIME);
      physics_steps_frame = 0;
      physics_us_frame = 0.f;
      while (physics_accumulator >= PHYSICS_DT) {
        play->update(current_input, PHYSICS_DT);
        m_world.step(PHYSICS_DT);
        physics_accumulator -= PHYSICS_DT;
        ++physics_steps_frame;
        physics_us_frame += m_world.getStats().step_time_us;
      }

      sf::Vector2f player_pos = play->getPosition();
      float play_dir = 0.0f;
      sf::Vector2f v_center;

      if (using_controller) {
        float u = sf::Joystick::getAxisPosition(active_joystick_id, sf::Joystick::Axis::U);
        float v = sf::Joystick::getAxisPosition(active_joystick_id, sf::Joystick::Axis::V);
        sf::Vector2f aim_dir(u, v);
        const float deadzone = 15.0f;

        if (aim_dir.lengthSquared() > deadzone * deadzone) {
          play_dir = get_angle(aim_dir);
          last_play_dir = play_dir;
        } else {
          play_dir = last_play_dir;
        }

        // Calculate a lookahead based on aim direction (length relative to right stick magnitude)
        sf::Vector2f lookahead = aim_dir * 1.5f; // scale aim_dir to match mouse feel
        v_center = sf::Vector2f((3 * player_pos.x + (player_pos.x + lookahead.x)) / 4.0f,
                                (3 * player_pos.y + (player_pos.y + lookahead.y)) / 4.0f);
      } else {
        sf::Vector2f mouse_pos =
            m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), view);
        play_dir = get_angle(mouse_pos - player_pos);
        last_play_dir = play_dir;
        v_center = sf::Vector2f((3 * player_pos.x + mouse_pos.x) / 4.0f,
                                (3 * player_pos.y + mouse_pos.y) / 4.0f);
      }

      // Update Camera View
      view.setCenter(max(v_center, minCenter));

      // Face the target
      play->setRotation(sf::degrees(play_dir));
    }

    // Render
    m_window.clear(sf::Color(50, 50, 50));  // Clear once per frame

    if (game_state == GameState::Menu) {
      if (startButton.isMouseOver(m_window)) {
        startButton.setFillColor(sf::Color(170, 170, 170));
      } else {
        startButton.setFillColor(sf::Color(200, 200, 200));
      }

      if (quitButton.isMouseOver(m_window)) {
        quitButton.setFillColor(sf::Color(170, 170, 170));
      } else {
        quitButton.setFillColor(sf::Color(200, 200, 200));
      }

      // Draw Menu (UI View)
      drawMenu();
    } else {
      drawWorld();
      if (Object::g_draw_collisions) drawStats(dt);

      // If paused, draw overlay and UI on top
      if (game_state == GameState::Paused) {
        m_window.setView(m_window.getDefaultView());
        m_window.draw(pauseOverlay);
        sf::Text pausedText(font, "Game Paused", SMALL_FONT_SIZE);
        pausedText.setFillColor(sf::Color::White);

        // Center text using screen coordinates
        sf::FloatRect textRect = pausedText.getLocalBounds();
        pausedText.setOrigin(
            sf::Vector2f(textRect.position.x + textRect.size.x / 2.0f,
                         textRect.position.y + textRect.size.y / 2.0f));
        pausedText.setPosition(sf::Vector2f(m_window.getSize().x / 2.0f,
                                            m_window.getSize().y / 2.0f));

        m_window.draw(pausedText);
      }
    }

    m_window.display();
  }
  delete[] m_p;
  m_world.clear();
  delete play;

  for (auto* obj : m_object_list) {
    delete obj;
  }
  m_object_list.clear();

  for (auto& layer : m_sprite_layer) {
    layer.clear();
  }

  return 0;
}
