#include "Button.h"

#include <SFML/Graphics.hpp>

Button::Button() {
  setSize(sf::Vector2f(100, 50));
  sf::RectangleShape::setPosition(sf::Vector2f(0, 0));
  setFillColor(sf::Color(200, 200, 200));
  setOutlineColor(sf::Color(220, 220, 220));
  setOutlineThickness(1.0f);
}

Button::Button(const sf::Vector2f& size, const sf::Vector2f& position,
               const std::string& text, const sf::Font& font,
               unsigned int characterSize)
    : sf::RectangleShape(size) {
  sf::RectangleShape::setPosition(position);
  setFillColor(sf::Color(200, 200, 200));
  setOutlineColor(sf::Color(220, 220, 220));
  setOutlineThickness(1.0f);
  m_buttonText = new sf::Text(font, text, characterSize);
  setText();
}

Button::~Button() { delete m_buttonText; }

void Button::setPosition(const sf::Vector2f& position) {
  if (m_buttonText != nullptr) {
    sf::RectangleShape::setPosition(position);
    setText();
  }
}

void Button::setText() {
  sf::Vector2f textPos;
  textPos.x = getPosition().x + (getGlobalBounds().size.x -
                                 m_buttonText->getGlobalBounds().size.x) *
                                    0.5f;
  textPos.y = getPosition().y + (getGlobalBounds().size.y -
                                 m_buttonText->getGlobalBounds().size.y) *
                                    0.25f;
  m_buttonText->setPosition(textPos);
}

void Button::setScale(sf::Vector2f scale) {
  sf::RectangleShape::setScale(scale);
  m_buttonText->setScale(scale);
  setText();
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  target.draw(static_cast<sf::RectangleShape>(*this), states);
  target.draw(*m_buttonText, states);
}

bool Button::isMouseOver(const sf::RenderWindow& window) const {
  sf::Vector2f mousePosition =
      window.mapPixelToCoords(sf::Mouse::getPosition(window));
  return getGlobalBounds().contains(mousePosition);
}
