#include <SFML/Graphics.hpp>
#include "Game.h"
#include "Player.h"
#include "utils.h"
#include <iostream>

Game::Game(int framerate)
{
  sf::Clock clock;
  sf::RenderWindow window { { 600u, 480u }, "Phantom Force" };
  window.setFramerateLimit(framerate);

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

  uint8_t input = 0;

  while (window.isOpen())
  {
    float frame = clock.getElapsedTime().asSeconds() * 60;
    clock.restart();

    for (auto event = sf::Event{}; window.pollEvent(event);)
    {

      if (event.type == sf::Event::Closed)
      {
        window.close();
      } else if (event.type == sf::Event::KeyPressed)
      {
        switch (event.key.code)
        {
        case sf::Keyboard::A:
          input |= 0b00000010; // Set bit 1 (left)
          break;
        case sf::Keyboard::D:
          input |= 0b00000001; // Set bit 0 (right)
          break;
        case sf::Keyboard::W:
          input |= 0b00001000; // Set bit 3 (up)
          break;
        case sf::Keyboard::S:
          input |= 0b00000100; // Set bit 2 (down)
          break;
        case sf::Keyboard::Escape:
          window.close();
          break;
        default:
          break;
        }
      } else if (event.type == sf::Event::KeyReleased)
      {
        switch (event.key.code)
        {
        case sf::Keyboard::A:
          input &= ~0b00000010; // Reset bit 1 (left)
          break;
        case sf::Keyboard::D:
          input &= ~0b00000001; // Reset bit 0 (right)
          break;
        case sf::Keyboard::W:
          input &= ~0b00001000; // Reset bit 3 (up)
          break;
        case sf::Keyboard::S:
          input &= ~0b00000100; // Reset bit 2 (down)
          break;
        default:
          break;
        }
      }
    }

    float e = get_angle(sf::Vector2f(sf::Mouse::getPosition(window)) - play.getPosition());

    window.clear();
    play.move(sf::Vector2f(3,3), frame, false, input);
    play.setRotation(e);

    for (int i = 3; i >= 1; i--)
		{
			for (int j = 0; j < game_sprites[i].size(); j++)
			{
				window.draw(*game_sprites[i][j]);
			}
		}
    window.display();
  }
}
