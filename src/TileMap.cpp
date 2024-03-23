#include "TileMap.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include "utils.h"

bool TileMap::loadTileset(const std::string& tileset, sf::Vector2u tileSize)
{
  if (!m_tileset.loadFromFile(tileset))
    return false;

  m_tileSize = tileSize;
  return (m_tileSize != sf::Vector2u(0, 0));
}

void TileMap::loadMap(const int* tiles, unsigned int mapWidth, unsigned int mapHeight, sf::View view)
{
  m_tiles.resize(mapWidth * mapHeight);
  for (int i = 0; i < mapWidth * mapHeight; i++)
    m_tiles[i] = tiles[i];

  m_mapSize = sf::Vector2u(mapWidth, mapHeight);
  sf::Vector2f chunk = view.getSize() * 5.0f;
  m_chunkSize = sf::Vector2u(chunk.x / m_tileSize.x + 1, chunk.y / m_tileSize.y + 1);

  m_vertices.setPrimitiveType(sf::Quads);
  m_vertices.resize(m_chunkSize.x * m_chunkSize.y * 4);
  m_center = view.getCenter();
}

// view_coord is the center coordinates of the view
void TileMap::loadVertexChunk(sf::Vector2f view_coord)
{
  /**
   * chunkLimit represents the furthest distance the view center
   * can to be from the chunk center before chunk is redrawn
   */
  float chunkLimit_x = m_chunkSize.x * m_tileSize.x * 0.5;
  float chunkLimit_y = m_chunkSize.y * m_tileSize.y * 0.5;

  if (len(m_center - view_coord) < len(sf::Vector2f(chunkLimit_x, chunkLimit_y)) * 0.3)
    return;
  

  std::cout << "(" << view_coord.x << ", " << view_coord.y << ")" << std::endl;
  std::cout << m_center.x << ", " << m_center.y << std::endl;

  m_center = view_coord;
  // t represents the topleft corner of the view
  float t_x = std::max(view_coord.x - chunkLimit_x, 0.0f);
  float t_y = std::max(view_coord.y - chunkLimit_y, 0.0f);

  flash(sf::Vector2i(t_x / m_tileSize.x, t_y / m_tileSize.y));
}

void TileMap::flash(sf::Vector2i t)
{
  // Populate the vertex array, with one quad per tile
  for (unsigned int i = t.x; i < std::min(m_chunkSize.x + t.x, m_mapSize.x); i++)
  {
    for (unsigned int j = t.y; j < std::min(m_chunkSize.y + t.y, m_mapSize.y); j++)
    {
      // Get the current tile number
      int tileNumber = m_tiles[i + j * m_mapSize.x];

      // Calculate its position in the tileset texture
      int tu = tileNumber % (m_tileset.getSize().x / m_tileSize.x);
      int tv = tileNumber / (m_tileset.getSize().x / m_tileSize.x);

      // Get a pointer to the current tile's quad
      sf::Vertex* quad = &m_vertices[(i - t.x + (j - t.y) * m_chunkSize.x) * 4];

      // Define corners
      quad[0].position = sf::Vector2f(i * m_tileSize.x, j * m_tileSize.y);
      quad[1].position = sf::Vector2f((i + 1) * m_tileSize.x, j * m_tileSize.y);
      quad[2].position = sf::Vector2f((i + 1) * m_tileSize.x, (j + 1) * m_tileSize.y);
      quad[3].position = sf::Vector2f(i * m_tileSize.x, (j + 1) * m_tileSize.y);

      // Define texture coordinates
      quad[0].texCoords = sf::Vector2f(tu * m_tileSize.x, tv * m_tileSize.y);
      quad[1].texCoords = sf::Vector2f((tu + 1) * m_tileSize.x, tv * m_tileSize.y);
      quad[2].texCoords = sf::Vector2f((tu + 1) * m_tileSize.x, (tv + 1) * m_tileSize.y);
      quad[3].texCoords = sf::Vector2f(tu * m_tileSize.x, (tv + 1) * m_tileSize.y);
    }
  }
}

void TileMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
  // Combine the TileMap's transform with the current RenderStates
  states.transform *= getTransform();

  // Set the TileMap's texture as the current texture in the RenderStates
  states.texture = &m_tileset;

  // Draw the visible tiles to the RenderTarget using the updated RenderStates
  target.draw(m_vertices, states);
}
