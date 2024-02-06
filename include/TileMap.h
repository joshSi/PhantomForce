#ifndef TILEMAP_H
#define TILEMAP_H
#include <SFML/Graphics.hpp>

class TileMap : public sf::Drawable, public sf::Transformable
{
public:
    bool loadTileset(const std::string& tileset, sf::Vector2u tileSize);
    void loadMap(const int* tiles, unsigned int mapWidth, unsigned int mapHeight, sf::View view);
    void loadVertexChunk(sf::Vector2f view_coord);
    // Flashes the chunk with tile v as the topleft corner
    void flash(sf::Vector2i v);

private:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

    sf::VertexArray m_vertices;  // The vertex array to draw the tiles onto the view
    sf::Vector2u m_chunkSize;    // The dimensions of a VertexArray chunk when rendered (in tiles)
    sf::Texture m_tileset;       // The tileset texture
    sf::Vector2u m_tileSize;     // The dimensions of each tile (in pixels)
    std::vector<int> m_tiles;    // The tile map
    sf::Vector2u m_mapSize;      // The dimensions of the entire tile map (in tiles)
    sf::Vector2f m_center;       // The center of the view since loadVertexChunk is last called
};

#endif