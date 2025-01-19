#include "Button.h"

#include <SFML/Graphics.hpp>

Button::Button(const sf::Vector2f& size, const sf::Vector2f& position,
               const std::string& text, const sf::Font& font,
               unsigned int characterSize)
    : sf::RectangleShape(size) {
  setPosition(position);
  buttonText.setString(text);
  buttonText.setFont(font);
  buttonText.setCharacterSize(characterSize);
  setText();
}

void Button::setText() {
  buttonText.setPosition(getPosition().x + getGlobalBounds().width / 2 -
                             buttonText.getGlobalBounds().width / 2,
                         getPosition().y + getGlobalBounds().height / 2 -
                             buttonText.getGlobalBounds().height / 2);
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  target.draw(buttonShape, states);
  target.draw(buttonText, states);
}

bool Button::isMouseOver(const sf::RenderWindow& window) const {
  sf::Vector2f mousePosition =
      window.mapPixelToCoords(sf::Mouse::getPosition(window));
  return getGlobalBounds().contains(mousePosition);
}
