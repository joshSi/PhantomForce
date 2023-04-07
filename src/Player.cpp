#include "Player.h"
#include "utils.h"
#include <iostream>

Player::Player():
	spd_vec(0,0)
{
	setScale(4, 4);
}

Player::Player(sf::Texture& tex, MoveStats* s):
	stat(s),
	spd_vec(0,0)
{
	setScale(4, 4);
	setTexture(tex);
}

void Player::move(sf::Vector2f velocity, float fr, bool sprint, uint8_t input)
{
		sf::Vector2f v(
			int((input & 0b00000001) != 0) - int((input & 0b00000010) != 0),
			int((input & 0b00000100) != 0) - int((input & 0b00001000) != 0)
	);

  if (len(v) != 0)
	  std::cout << fr << " i: " << int(input) << std::endl;

	if (len(v))
		spd_vec += v * (stat->accel * fr / len(v));

	if (len(spd_vec)) {
		spd_vec *= powf(stat->fric, fr);
		if (len(spd_vec) < 0.05f)
			spd_vec *= 0.f;
	}

	if (len(spd_vec) > stat->max_spd)
		spd_vec *= (stat->max_spd / len(spd_vec));
	sf::Sprite::move(spd_vec*fr);
}
