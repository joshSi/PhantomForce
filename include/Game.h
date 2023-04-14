#ifndef GAME_H
#define GAME_H
#include <SFML/Graphics.hpp>

class Game
{
public:
	Game(int framerate = 60);
	Game(const Game&) = delete;
  void operator=(const Game&) = delete;
	~Game();

	void pollEvents();
private:
	sf::RenderWindow* m_window;
	uint8_t m_input;
};

#endif
