#include <SFML/Graphics.hpp>
#include "Game.h"
#include "Player.h"
#include "TileMap.h"
#include "utils.h"
#include <iostream>

Game::Game(int framerate) : m_input(0)
{
  sf::Clock clock;
  m_window = new sf::RenderWindow { { 1000u, 800u }, "Phantom Force" };
  m_window->setFramerateLimit(framerate);

  sf::Texture tex;
  sf::Texture background_tex;

  std::vector<sf::Sprite*> game_sprites[4];
  MoveStats def({5.0f, 0.5f, 0.8f, 1.0f});


  // Viewport of 250 x 200, 1/4 of the original window 1000 x 800
  sf::View view = m_window->getView();
  sf::Vector2f minCenter = sf::Vector2f(125.f, 100.f);
  view.zoom(0.25f);
  m_window->setView(view);

	if (!(tex.loadFromFile("assets/player.png")))
	{
		std::cerr << "Loading texture failed" << std::endl;
	}

  Player play = Player(tex, &def);
  play.setPosition(10, 10);
  game_sprites[1].push_back(&play);

  // Load the background texture and create a sprite for it
  if (!(background_tex.loadFromFile("assets/background.png")))
	{
		std::cerr << "Loading background failed" << std::endl;
	}
  background_tex.setRepeated(true);
  // sf::Sprite background(background_tex);

  TileMap background_map;
  int* p = new int[10000];
  for (int i = 0; i < 10000; i++)
    p[i] = i%4;
  p[130] = 2;
  p[420] = 0;
  background_map.loadTileset("assets/background.png", sf::Vector2u(32, 32));
  background_map.loadMap(p, 100, 100, view);
  background_map.flash(sf::Vector2i(0, 0));

  while (m_window->isOpen())
  {
    float frame = clock.restart().asSeconds() * 60;

    pollEvents();

    sf::Vector2f mouse_pos = m_window->mapPixelToCoords(sf::Mouse::getPosition(*m_window), m_window->getView());
    sf::Vector2f player_pos = play.getPosition();
    float play_dir = get_angle(sf::Vector2f(mouse_pos) - player_pos);

    m_window->clear();

    view = m_window->getView();
    sf::Vector2f v_center((3 * player_pos.x + mouse_pos.x) / 4.0f, (3 * player_pos.y + mouse_pos.y) / 4.0f);
    view.setCenter(max(v_center, minCenter));
    m_window->setView(view);
    background_map.loadVertexChunk(v_center);

    play.move(sf::Vector2f(3,3), frame, false, m_input);
    play.setRotation(play_dir);
    m_window->draw(background_map);

    for (int i = 3; i >= 1; i--)
		{
			for (int j = 0; j < game_sprites[i].size(); j++)
			{
				m_window->draw(*game_sprites[i][j]);
			}
		}
    m_window->display();
  }
}

Game::~Game()
{
  delete m_window;
}

void Game::pollEvents()
{
  sf::Event event;
  while (m_window->pollEvent(event))
  {
    if (event.type == sf::Event::Closed)
    {
      m_window->close();
    } else if (event.type == sf::Event::KeyPressed)
    {
      switch (event.key.code)
      {
      case sf::Keyboard::A:
        m_input |= 0b00000010; // Set bit 1 (left)
        break;
      case sf::Keyboard::D:
        m_input |= 0b00000001; // Set bit 0 (right)
        break;
      case sf::Keyboard::W:
        m_input |= 0b00001000; // Set bit 3 (up)
        break;
      case sf::Keyboard::S:
        m_input |= 0b00000100; // Set bit 2 (down)
        break;
      case sf::Keyboard::Escape:
        m_window->close();
        break;
      default:
        break;
      }
    } else if (event.type == sf::Event::KeyReleased)
    {
      switch (event.key.code)
      {
      case sf::Keyboard::A:
        m_input &= ~0b00000010; // Reset bit 1 (left)
        break;
      case sf::Keyboard::D:
        m_input &= ~0b00000001; // Reset bit 0 (right)
        break;
      case sf::Keyboard::W:
        m_input &= ~0b00001000; // Reset bit 3 (up)
        break;
      case sf::Keyboard::S:
        m_input &= ~0b00000100; // Reset bit 2 (down)
        break;
      default:
        break;
      }
    }
  }
}
