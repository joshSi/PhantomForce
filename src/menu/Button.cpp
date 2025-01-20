#include "Button.h"

#include <SFML/Graphics.hpp>

Button::Button(const sf::Vector2f& size, const sf::Vector2f& position,
               const std::string& text, const sf::Font& font,
               unsigned int characterSize)
    : sf::RectangleShape(size), m_buttonText(font) {
  setPosition(position);
  m_buttonText.setString(text);
  m_buttonText.setCharacterSize(characterSize);
  setText();
}

void Button::setText() {
  sf::Vector2f textPos = getPosition();
  textPos.x +=
      getGlobalBounds().size.x / 2 - m_buttonText.getGlobalBounds().size.x / 2;
  textPos.y +=
      getGlobalBounds().size.y / 2 - m_buttonText.getGlobalBounds().size.y / 2;
  m_buttonText.setPosition(textPos);
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  target.draw(m_buttonText, states);
}

bool Button::isMouseOver(const sf::RenderWindow& window) const {
  sf::Vector2f mousePosition =
      window.mapPixelToCoords(sf::Mouse::getPosition(window));
  return getGlobalBounds().contains(mousePosition);
}
