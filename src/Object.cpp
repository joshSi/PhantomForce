#include "Object.h"

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
  // TODO: Collision detection
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
