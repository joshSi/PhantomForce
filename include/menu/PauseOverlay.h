#ifndef PAUSE_OVERLAY_H
#define PAUSE_OVERLAY_H
#include <SFML/Graphics.hpp>

#include "Button.h"

class PauseOverlay : public sf::Drawable {
 public:
  PauseOverlay();
  PauseOverlay(const sf::Font font);

 private:
  sf::Font m_font;

  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif