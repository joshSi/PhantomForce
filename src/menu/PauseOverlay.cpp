#include "menu/PauseOverlay.h"

PauseOverlay::PauseOverlay() {}

PauseOverlay::PauseOverlay(const sf::Font font) : m_font(font) {}

void PauseOverlay::draw(sf::RenderTarget& target,
                        sf::RenderStates states) const {
  // Fill screen with overlay
  sf::RectangleShape overlay;
  overlay.setSize(sf::Vector2f(target.getSize().x, target.getSize().y));
  overlay.setPosition(target.mapPixelToCoords(sf::Vector2i(0, 0)));
  overlay.setFillColor(sf::Color(0, 0, 0, 128));
  target.draw(overlay);

  // TODO: Draw settings
  sf::Text titleText;
  titleText.setString("Settings");
  titleText.setFont(m_font);
  titleText.setCharacterSize(8);
  titleText.setPosition(20, 20);

  target.draw(titleText, states);
  Button settingsButton(sf::Vector2f(100, 50), sf::Vector2f(20, 20), "Settings",
                        m_font);
  target.draw(settingsButton, states);
}
