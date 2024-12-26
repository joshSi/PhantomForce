#ifndef BUTTON_H
#define BUTTON_H
#include <SFML/Graphics.hpp>

enum class ButtonState { NORMAL, HOVER, PRESSED };

class Button {
 public:
  Button(const sf::Vector2f& size, const sf::Vector2f& position,
         const std::string& text, const sf::Font& font,
         unsigned int characterSize = 30);
  void draw(sf::RenderWindow& window) const;
  bool isMouseOver(const sf::RenderWindow& window) const;

 private:
  sf::RectangleShape buttonShape;
  sf::Text buttonText;
  ButtonState state = ButtonState::NORMAL;
};

#endif
