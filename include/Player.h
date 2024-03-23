#ifndef PLAYER_H
#define PLAYER_H
#include <SFML/Graphics.hpp>

struct MoveStats {
	float max_spd;
	float accel;
	float fric;
	float reload_rate;
};

class Player : public sf::Sprite
{
	public:
		Player();
		Player(sf::Texture& tex, MoveStats* s);
		void move(sf::Vector2f velocity, float fr, bool sprint, uint8_t input);

	private:
		sf::Vector2f spd_vec;
		MoveStats* stat;
};

#endif
