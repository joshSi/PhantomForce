#include "Button.h"

#include <SFML/Graphics.hpp>

Button::Button() {
  setSize(sf::Vector2f(100, 50));
  setPosition(sf::Vector2f(0, 0));
  setFillColor(sf::Color::White);
  setOutlineColor(sf::Color::Black);
  setOutlineThickness(1.0f);
}

Button::Button(const sf::Vector2f& size, const sf::Vector2f& position,
               const std::string& text, const sf::Font& font,
               unsigned int characterSize)
    : sf::RectangleShape(size) {
  setPosition(position);
  m_buttonText = new sf::Text(font, text, characterSize);
  setText();
}

Button::~Button() { delete m_buttonText; }

void Button::setText() {
  sf::Vector2f textPos = getPosition();
  textPos.x +=
      getGlobalBounds().size.x / 2 - m_buttonText->getGlobalBounds().size.x / 2;
  textPos.y +=
      getGlobalBounds().size.y / 2 - m_buttonText->getGlobalBounds().size.y / 2;
  m_buttonText->setPosition(textPos);
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  target.draw(*m_buttonText, states);
}

bool Button::isMouseOver(const sf::RenderWindow& window) const {
  sf::Vector2f mousePosition =
      window.mapPixelToCoords(sf::Mouse::getPosition(window));
  return getGlobalBounds().contains(mousePosition);
}
