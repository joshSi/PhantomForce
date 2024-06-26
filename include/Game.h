#ifndef GAME_H
#define GAME_H
#include <SFML/Graphics.hpp>

#include "Object.h"

class Game {
 public:
  Game(int framerate = 60);
  Game(const Game&) = delete;
  void operator=(const Game&) = delete;
  ~Game();

  void pollEvents();

 private:
  sf::Clock m_clock;
  sf::RenderWindow* m_window;
  // Up to 4 drawing layers
  std::vector<sf::Sprite*> m_sprite_layer[4];
  std::vector<Object*> m_object_list;
  uint8_t m_input;
};

#endif
