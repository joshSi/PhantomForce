#include <SFML/Graphics.hpp>
#include "Game.h"
#include "Player.h"
#include "utils.h"
#include <iostream>

Game::Game(int framerate) : m_input(0)
{
  sf::Clock clock;
  m_window = new sf::RenderWindow { { 600u, 480u }, "Phantom Force" };
  m_window->setFramerateLimit(framerate);

  sf::Texture tex;

  std::vector<sf::Sprite*> game_sprites[4];
  MoveStats def({9.0f, 1.0f, 0.9f, 1.0f});

	if (!(tex.loadFromFile("assets/player.png")))
	{
		std::cerr << "Loading texture failed" << std::endl;
	}

  Player play = Player(tex, &def);
  play.setOrigin(32, 32);
  game_sprites[1].push_back(&play);

  sf::Sprite spr;
  spr.setTexture(tex);
  spr.setPosition(200, 200);

  while (m_window->isOpen())
  {
    float frame = clock.getElapsedTime().asSeconds() * 60;
    clock.restart();

    pollEvents();
    std::cout << m_input << std::endl;
    
    float e = get_angle(sf::Vector2f(sf::Mouse::getPosition(*m_window)) - play.getPosition());

    m_window->clear();
    play.move(sf::Vector2f(3,3), frame, false, m_input);
    play.setRotation(e);

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
