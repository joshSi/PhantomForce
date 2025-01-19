#include "Game.h"

#include <SFML/Graphics.hpp>

#include "Player.h"
#include "TileMap.h"
#include "platform_utils.h"
#include "utils.h"

Game::Game(int framerate) : m_input(0) {
  const std::string resourcePath = getResourcePath();
  m_window = new sf::RenderWindow{{1000u, 800u}, "Phantom Force"};
  m_window->setFramerateLimit(framerate);

  sf::Texture tex;
  sf::Texture crate_tex;
  sf::Font font;

  MoveStats def({5.0f, 0.5f, 0.8f, 1.0f});

  // Viewport of 250 x 200, 1/4 of the original window 1000 x 800
  sf::View view = m_window->getView();
  sf::Vector2f minCenter = sf::Vector2f(125.f, 100.f);
  view.zoom(0.25f);
  m_window->setView(view);

  if (!(tex.loadFromFile(resourcePath + "player.png"))) {
    printf("Loading texture failed\n");
  }
  if (!(crate_tex.loadFromFile(resourcePath + "crate.png"))) {
    printf("Loading texture failed\n");
  }

  if (!font.loadFromFile(resourcePath + "EBB.ttf")) {
    printf("Loading font failed\n");
  }
  m_pause_overlay = PauseOverlay(font);

  Player play = Player(tex, &def, 6.f);
  play.setPosition(10, 10);
  m_sprite_layer[1].push_back(&play);
  play.setObjects(&m_object_list);
  Circle c = Circle(tex, 20.f);
  c.setPosition(160, 40);
  m_object_list.push_back(&c);
  m_sprite_layer[1].push_back(&c);
  Rectangle r = Rectangle(crate_tex, sf::Vector2f(40.f, 400.f));
  r.setPosition(90, 240);
  m_object_list.push_back(&r);
  m_sprite_layer[1].push_back(&r);

  TileMap background_map;
  int* p = new int[10000];
  for (int i = 0; i < 10000; i++) p[i] = i % 4;
  p[101] = 15;
  p[102] = 16;
  p[203] = 15;
  p[302] = 15;
  if (!background_map.loadTileset(resourcePath + "background.png",
                                  sf::Vector2u(32, 32)))
    printf("Loading tileset failed\n");
  background_map.loadMap(p, 100, 100, view);
  background_map.flash(sf::Vector2i(0, 0));

  while (m_window->isOpen()) {
    float frame = m_clock.restart().asSeconds() * 60;

    if (m_paused) {
      pause();
    }

    pollEvents();

    sf::Vector2f mouse_pos = m_window->mapPixelToCoords(
        sf::Mouse::getPosition(*m_window), m_window->getView());
    sf::Vector2f player_pos = play.getPosition();
    float play_dir = get_angle(sf::Vector2f(mouse_pos) - player_pos);

    m_window->clear();

    view = m_window->getView();
    sf::Vector2f v_center((3 * player_pos.x + mouse_pos.x) / 4.0f,
                          (3 * player_pos.y + mouse_pos.y) / 4.0f);
    view.setCenter(max(v_center, minCenter));
    m_window->setView(view);
    background_map.loadVertexChunk(v_center);

    play.move(sf::Vector2f(3, 3), frame, false, m_input);
    play.setRotation(play_dir);
    m_window->draw(background_map);

    if (Object::g_draw_collisions) {
      play.drawCollision(m_window);
      for (int i = 0; i < m_object_list.size(); i++) {
        m_object_list[i]->drawCollision(m_window);
      }
    }
    for (int i = 3; i >= 1; i--) {
      for (int j = 0; j < m_sprite_layer[i].size(); j++) {
        m_window->draw(*m_sprite_layer[i][j]);
      }
    }
    m_window->display();
  }
}

Game::~Game() { delete m_window; }

void Game::pollEvents() {
  sf::Event event;
  while (m_window->pollEvent(event)) {
    if (event.type == sf::Event::Closed) {
      m_window->close();
    } else if (event.type == sf::Event::KeyPressed) {
      switch (event.key.code) {
        case sf::Keyboard::A:
          m_input |= 0b00000010;  // Set bit 1 (left)
          break;
        case sf::Keyboard::D:
          m_input |= 0b00000001;  // Set bit 0 (right)
          break;
        case sf::Keyboard::W:
          m_input |= 0b00001000;  // Set bit 3 (up)
          break;
        case sf::Keyboard::S:
          m_input |= 0b00000100;  // Set bit 2 (down)
          break;
        case sf::Keyboard::Space:
          Object::g_draw_collisions = !Object::g_draw_collisions;
          break;
        default:
          break;
      }
    } else if (event.type == sf::Event::KeyReleased) {
      switch (event.key.code) {
        case sf::Keyboard::A:
          m_input &= ~0b00000010;  // Reset bit 1 (left)
          break;
        case sf::Keyboard::D:
          m_input &= ~0b00000001;  // Reset bit 0 (right)
          break;
        case sf::Keyboard::W:
          m_input &= ~0b00001000;  // Reset bit 3 (up)
          break;
        case sf::Keyboard::S:
          m_input &= ~0b00000100;  // Reset bit 2 (down)
          break;
        case sf::Keyboard::Escape:
          // Pause the game and open the settings overlay
          m_paused = !m_paused;
        default:
          break;
      }
    }
  }
}

void Game::pause() {
  if (m_window != nullptr) {
    m_window->draw(m_pause_overlay);
    m_window->display();
  }
  // Wait for the overlay to be closed
  while (m_window->isOpen()) {
    pollEvents();
    if (!m_paused) {
      return;
    }
  }
}
