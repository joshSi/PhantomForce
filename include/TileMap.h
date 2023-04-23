#ifndef TILEMAP_H
#define TILEMAP_H
#include <SFML/Graphics.hpp>

class TileMap : public sf::Drawable, public sf::Transformable
{
public:
    bool load(const std::string& tileset, sf::Vector2u tileSize, const int* tiles, unsigned int width, unsigned int height);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    sf::VertexArray m_vertices;  // The vertex array used to draw the tiles
    sf::Texture m_tileset;       // The tileset texture
    sf::Vector2u m_tileSize;     // The size of each tile
    sf::Vector2u m_mapSize;      // The size of the tile map
};

#endif