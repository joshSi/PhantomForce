#ifndef PAUSE_OVERLAY_H
#define PAUSE_OVERLAY_H
#include <SFML/Graphics.hpp>

class PauseOverlay : public sf::Drawable {
private:
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
    // Fill screen with overlay
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(target.getSize().x, target.getSize().y));
    overlay.setPosition(target.mapPixelToCoords(sf::Vector2i(0, 0)));
    overlay.setFillColor(sf::Color(0, 0, 0, 128));
    target.draw(overlay);

    // TODO: Draw settings
  }
};

#endif