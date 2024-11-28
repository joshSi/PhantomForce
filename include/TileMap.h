#ifndef TILEMAP_H
#define TILEMAP_H
#include <SFML/Graphics.hpp>

class TileMap : public sf::Drawable {
 public:
  bool loadTileset(const std::string& tileset, sf::Vector2u tileSize);
  void loadMap(const int* tiles, unsigned int mapWidth, unsigned int mapHeight,
               sf::View view);
  void loadVertexChunk(sf::Vector2f view_coord);
  // Flashes the chunk with tile v as the topleft corner
  void flash(sf::Vector2i v);

 private:
  virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

  // The vertex array to draw the tiles onto the view
  sf::VertexArray m_vertices;
  // The dimensions of a VertexArray chunk when rendered (in tiles)
  sf::Vector2u m_chunkSize;
  // The tileset texture
  sf::Texture m_tileset;
  // The dimensions of each tile (in pixels)
  sf::Vector2u m_tileSize;
  // The tile map
  std::vector<int> m_tiles;
  // The dimensions of the entire tile map (in tiles)
  sf::Vector2u m_mapSize;
  // The center of the view since loadVertexChunk is last called
  sf::Vector2f m_center;
};

#endif
