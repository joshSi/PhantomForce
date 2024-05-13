#include "Player.h"
#include "utils.h"

Player::Player():
	spd_vec(0,0)
{
}

Player::Player(sf::Texture& tex, MoveStats* s):
	stat(s),
	spd_vec(0,0)
{
	setTexture(tex);
	sf::Vector2f player_size = sf::Vector2f(getLocalBounds().width, getLocalBounds().height);
	setOrigin( player_size.x / 2, player_size.y / 2);
}

void Player::move(sf::Vector2f velocity, float fr, bool sprint, uint8_t input)
{
	sf::Vector2f v(
		int((input & 0b00000001) != 0) - int((input & 0b00000010) != 0),
		int((input & 0b00000100) != 0) - int((input & 0b00001000) != 0)
	);

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
