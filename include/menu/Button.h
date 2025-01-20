#ifndef BUTTON_H
#define BUTTON_H
#include <SFML/Graphics.hpp>

enum class ButtonState { NORMAL, HOVER, PRESSED };

class Button : public sf::RectangleShape {
 public:
  Button(const sf::Text text) : sf::RectangleShape(), m_buttonText(text) {
    setText();
  }
  Button(const sf::Text text, const sf::Vector2f& size)
      : sf::RectangleShape(size), m_buttonText(text) {
    setText();
  }
  Button(const sf::Text text, const sf::Vector2f& size,
         const sf::Vector2f& position)
      : sf::RectangleShape(size), m_buttonText(text) {
    setPosition(position);
    setText();
  }

  Button(const sf::Vector2f& size, const sf::Vector2f& position,
         const std::string& text, const sf::Font& font,
         unsigned int characterSize = 30);

  bool isMouseOver(const sf::RenderWindow& window) const;

 private:
  sf::RectangleShape m_buttonShape;
  sf::Text m_buttonText;
  ButtonState state = ButtonState::NORMAL;
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
  void setText();
};

#endif
