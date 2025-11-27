#include "Game.h"

#include <SFML/Graphics.hpp>

#include "Player.h"
#include "TileMap.h"
#include "platform_utils.h"
#include "utils.h"

int runGame(int framerate = 60) {
  GameState game_state = GameState::Menu;
  int* m_p;  // Pointer to dynamically allocated array for tile map data
  sf::Clock m_clock;
  // Up to 4 drawing layers
  std::vector<sf::Sprite*> m_sprite_layer[4];
  std::vector<Object*> m_object_list;
  uint8_t m_input;
  bool close = false;

  const std::string resourcePath = getResourcePath();
  sf::RenderWindow m_window(sf::VideoMode({1000, 800}), "Phantom Force");
  m_window.setFramerateLimit(framerate);

  sf::Texture tex;
  sf::Texture crate_tex;
  sf::Font font;

  MoveStats def({5.0f, 0.5f, 0.8f, 1.0f});

  // Viewport of 250 x 200, 1/4 of the original window 1000 x 800
  sf::View view = m_window.getView();
  sf::Vector2f minCenter = sf::Vector2f(125.f, 100.f);
  view.zoom(0.25f);
  m_window.setView(view);

  if (!(tex.loadFromFile(resourcePath + "player.png"))) {
    printf("Loading texture failed\n");
  }
  if (!(crate_tex.loadFromFile(resourcePath + "crate.png"))) {
    printf("Loading texture failed\n");
  }
  if (!font.openFromFile(resourcePath + "EBB.ttf")) {
    printf("Loading font failed\n");
  }

  Player* play = new Player(tex, &def, 6.f);
  play->setPosition(sf::Vector2f(10, 10));
  m_sprite_layer[1].push_back(play);
  play->setObjects(&m_object_list);
  Circle* c = new Circle(tex, 20.f);
  c->setPosition(sf::Vector2f(160, 40));
  m_object_list.push_back(c);
  m_sprite_layer[1].push_back(c);
  Rectangle* r = new Rectangle(crate_tex, sf::Vector2f(40.f, 400.f));
  r->setPosition(sf::Vector2f(90, 240));
  m_object_list.push_back(r);
  m_sprite_layer[1].push_back(r);

  TileMap background_map;
  m_p = new int[10000];
  for (int i = 0; i < 10000; i++) m_p[i] = i % 4;
  m_p[101] = 15;
  m_p[102] = 16;
  m_p[203] = 15;
  m_p[302] = 15;
  if (!background_map.loadTileset(resourcePath + "background.png",
                                  sf::Vector2u(32, 32)))
    printf("Loading tileset failed\n");
  background_map.loadMap(m_p, 100, 100, view);
  background_map.flash(sf::Vector2i(0, 0));

  while (m_window.isOpen()) {
    float frame = m_clock.restart().asSeconds() * 60;

    // Input handling
    while (const std::optional event = m_window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        close = true;
      } else if (const sf::Event::KeyPressed* keyPressed =
                     event->getIf<sf::Event::KeyPressed>()) {
        switch (keyPressed->scancode) {
          case sf::Keyboard::Scancode::A:
            m_input |= 0b00000010;  // Set bit 1 (left)
            break;
          case sf::Keyboard::Scancode::D:
            m_input |= 0b00000001;  // Set bit 0 (right)
            break;
          case sf::Keyboard::Scancode::W:
            m_input |= 0b00001000;  // Set bit 3 (up)
            break;
          case sf::Keyboard::Scancode::S:
            m_input |= 0b00000100;  // Set bit 2 (down)
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
            m_input &= ~0b00000010;  // Reset bit 1 (left)
            break;
          case sf::Keyboard::Scancode::D:
            m_input &= ~0b00000001;  // Reset bit 0 (right)
            break;
          case sf::Keyboard::Scancode::W:
            m_input &= ~0b00001000;  // Reset bit 3 (up)
            break;
          case sf::Keyboard::Scancode::S:
            m_input &= ~0b00000100;  // Reset bit 2 (down)
            break;
          [[unlikely]] case sf::Keyboard::Scancode::Escape:
            game_state = game_state == GameState::Playing ? GameState::Paused
                                                          : GameState::Playing;
            break;
          default:
            break;
        }
      } else if (const sf::Event::MouseButtonPressed* mousePressed =
                     event->getIf<sf::Event::MouseButtonPressed>()) {
        printf("Mouse button pressed: %d at (%d, %d)\n",
               static_cast<int>(mousePressed->button), mousePressed->position.x,
               mousePressed->position.y);
      }
    }

    // Game state
    if (game_state == GameState::Paused) {
      m_window.clear(sf::Color(50, 50, 50));
      sf::Text pausedText(font, "Game Paused", 50);
      pausedText.setFillColor(sf::Color::White);
      pausedText.setPosition(
          sf::Vector2f(m_window.getView().getCenter().x -
                           pausedText.getGlobalBounds().size.x / 2,
                       m_window.getView().getCenter().y -
                           pausedText.getGlobalBounds().size.y / 2));
      m_window.draw(pausedText);
    } else if (game_state == GameState::Menu) {
    } else if (game_state == GameState::Playing) [[likely]] {
      sf::Vector2f mouse_pos = m_window.mapPixelToCoords(
          sf::Mouse::getPosition(m_window), m_window.getView());
      sf::Vector2f player_pos = play->getPosition();
      float play_dir = get_angle(sf::Vector2f(mouse_pos) - player_pos);
      m_window.clear();

      view = m_window.getView();
      sf::Vector2f v_center((3 * player_pos.x + mouse_pos.x) / 4.0f,
                            (3 * player_pos.y + mouse_pos.y) / 4.0f);
      view.setCenter(max(v_center, minCenter));
      m_window.setView(view);
      background_map.loadVertexChunk(v_center);

      play->move(sf::Vector2f(3, 3), frame, false, m_input);
      play->setRotation(sf::degrees(play_dir));
      m_window.draw(background_map);

      if (Object::g_draw_collisions) {
        play->drawCollision(&m_window);
        for (int i = 0; i < m_object_list.size(); i++) {
          m_object_list[i]->drawCollision(&m_window);
        }
      }
      for (int i = 3; i >= 1; i--) {
        for (int j = 0; j < m_sprite_layer[i].size(); j++) {
          m_window.draw(*m_sprite_layer[i][j]);
        }
      }
    }

    m_window.display();

    if (close) {
      m_window.close();
    }
  }
  delete[] m_p;

  for (auto& layer : m_sprite_layer) {
    for (auto* sprite : layer) {
      delete sprite;
    }
    layer.clear();
  }
  m_object_list.clear();
  printf("Game closed\n");

  return 0;
}
