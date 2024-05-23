#include "Player.h"
#include "utils.h"

Player::Player(sf::Texture& tex, MoveStats* s):
	m_stat(s)
{
	setTexture(tex);
	sf::Vector2f player_size = sf::Vector2f(getLocalBounds().width, getLocalBounds().height);
	setOrigin( player_size.x / 2, player_size.y / 2);
	// radius = player_size.x / 2;
}

void Player::move(sf::Vector2f velocity, float fr, bool sprint, uint8_t input)
{
	sf::Vector2f v(
		int((input & 0b00000001) != 0) - int((input & 0b00000010) != 0),
		int((input & 0b00000100) != 0) - int((input & 0b00001000) != 0)
	);

	if (len(v))
		spd_vec += v * (m_stat->accel * fr / len(v));

	if (len(spd_vec))
	{
		spd_vec *= powf(m_stat->fric, fr);
		if (len(spd_vec) < 0.05f)
			spd_vec *= 0.f;
	}

	if (len(spd_vec) > m_stat->max_spd)
		spd_vec *= (m_stat->max_spd / len(spd_vec));
	
	checkCollision();
	sf::Sprite::move(spd_vec*fr);
}

void Player::checkCollision()
{
	// TODO: Collision detection
}
