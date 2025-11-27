#include "menu/PauseOverlay.h"

PauseOverlay::PauseOverlay() {}

PauseOverlay::PauseOverlay(const sf::Font font) : m_font(font) {
  settingsButton =
      Button(sf::Vector2f(100, 50), sf::Vector2f(20, 20), "Settings", m_font);
}

PauseOverlay::~PauseOverlay() { printf("PauseOverlay destroyed\n"); }

void PauseOverlay::draw(sf::RenderTarget& target,
                        sf::RenderStates states) const {
  // Fill screen with overlay
  sf::RectangleShape overlay;
  overlay.setSize(sf::Vector2f(target.getSize().x, target.getSize().y));
  overlay.setPosition(target.mapPixelToCoords(sf::Vector2i(0, 0)));
  overlay.setFillColor(sf::Color(0, 0, 0, 128));
  target.draw(overlay);

  // TODO: Draw settings
  sf::Text titleText(m_font);
  titleText.setString("Settings 1");
  titleText.setCharacterSize(8);
  titleText.setPosition(sf::Vector2f(20, 20));

  target.draw(titleText, states);
  target.draw(settingsButton, states);
}
