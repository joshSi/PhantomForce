#ifndef GAME_H
#define GAME_H
#include <SFML/Graphics.hpp>

#include "Object.h"
#include "PauseOverlay.h"

int runGame(int framerate);

enum class GameState { Playing, Paused, Menu };

#endif
