#include "Button.h"

#include <SFML/Graphics.hpp>

Button::Button(const sf::Vector2f& size, const sf::Vector2f& position,
               const std::string& text, const sf::Font& font,
               unsigned int characterSize)
    : buttonShape(size), buttonText(text, font, characterSize) {
  buttonShape.setPosition(position);
  buttonText.setPosition(
      position.x + size.x / 2 - buttonText.getGlobalBounds().width / 2,
      position.y + size.y / 2 - buttonText.getGlobalBounds().height / 2);
}

void Button::draw(sf::RenderWindow& window) const {
  window.draw(buttonShape);
  window.draw(buttonText);
}
bool Button::isMouseOver(const sf::RenderWindow& window) const {
  sf::Vector2f mousePosition =
      window.mapPixelToCoords(sf::Mouse::getPosition(window));
  return buttonShape.getGlobalBounds().contains(mousePosition);
}
