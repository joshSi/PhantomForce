#ifndef BUTTON_H
#define BUTTON_H
#include <SFML/Graphics.hpp>

enum class ButtonState { NORMAL, HOVER, PRESSED };

class Button : public sf::RectangleShape {
 public:
  Button();

  Button(const sf::Vector2f& size, const sf::Vector2f& position,
         const std::string& text, const sf::Font& font,
         unsigned int characterSize = 30);

  ~Button();

  bool isMouseOver(const sf::RenderWindow& window) const;

  void setState(ButtonState state) { this->state = state; }
  void setScale(sf::Vector2f scale);
  void setPosition(const sf::Vector2f& position);

 private:
  sf::Text* m_buttonText;
  ButtonState state = ButtonState::NORMAL;
  void (*m_clickAction)(sf::Vector2f, Button&);
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
  void setText();
};

#endif
