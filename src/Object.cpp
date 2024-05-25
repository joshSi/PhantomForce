#include "Object.h"
#include "utils.h"

void Circle::collide(Object *obj)
{
  if (dynamic_cast<Circle*>(obj)) {
    collide(static_cast<Circle*>(obj));
  } else if (dynamic_cast<Rectangle*>(obj)) {
    collide(static_cast<Rectangle*>(obj));
  }
}

void Rectangle::collide(Object *obj)
{
  if (dynamic_cast<Circle*>(obj)) {
    collide(static_cast<Circle*>(obj));
  } else if (dynamic_cast<Rectangle*>(obj)) {
    collide(static_cast<Rectangle*>(obj));
  }
}

void Circle::collide(Circle *obj)
{
  float dist = len(obj->getPosition() - getPosition());
  if (len(obj->getPosition() - getPosition()) < obj->getRadius() + m_radius) {
    if (obj->m_mass == 0) {
      const float weight = (obj->getRadius() + m_radius) / len(m_spd_vec);
      // perpendicular point forms right triangle between tangent pos & obj->pos
      const sf::Vector2f perpendicular(obj->getPosition().x - weight*m_spd_vec.y, obj->getPosition().y + weight*m_spd_vec.x);
      // direction of m_spd_vec, tangent of circles
      const float length = sqrtf(pow(obj->getRadius() + m_radius, 2) - len(perpendicular - obj->getPosition()));

      setPosition(perpendicular.x + length * m_spd_vec.x/len(m_spd_vec), perpendicular.y - length * m_spd_vec.y/len(m_spd_vec));
      m_spd_vec = sf::Vector2f(0, 0);
    }
  }
}

void Circle::collide(Rectangle *obj)
{
  // TODO: Collision detection
}

void Rectangle::collide(Circle *obj)
{
  // TODO: Collision detection
}

void Rectangle::collide(Rectangle *obj)
{
  // TODO: Collision detection
}
