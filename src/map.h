#ifndef MAP_H
#define MAP_H

#include <array>
#include <cstddef>
#include <functional>
#include <ncurses.h>
#include <string>
#include <unordered_map>

// Each chunk is 64×64 tiles; world is 4×4 chunks → 256×256
constexpr int CHUNK_SIZE = 64;
constexpr int MAP_CHUNKS_X = 4;
constexpr int MAP_CHUNKS_Y = 4;
constexpr int WORLD_WIDTH  = MAP_CHUNKS_X * CHUNK_SIZE; // 256
constexpr int WORLD_HEIGHT = MAP_CHUNKS_Y * CHUNK_SIZE; // 256

// Block type codes stored in the TYPES section of each chunk file
enum BlockType : unsigned char {
  BLOCK_NORMAL  = 0, // grass, road, floor — free movement
  BLOCK_SOLID   = 1, // walls, buildings — no pass-through
  BLOCK_WATER   = 2, // slow swim
  BLOCK_DOOR    = 3, // closed door — solid until opened with 'e'
  BLOCK_DOOR_OPEN = 4 // open door — passable (runtime only)
};

struct Chunk {
  int chunkX = 0;
  int chunkY = 0;

  std::array<std::array<char, CHUNK_SIZE>, CHUNK_SIZE> glyphs{};
  std::array<std::array<short, CHUNK_SIZE>, CHUNK_SIZE> colors{};
  std::array<std::array<unsigned char, CHUNK_SIZE>, CHUNK_SIZE> types{};
};

struct ChunkKey {
  int x, y;
  bool operator==(const ChunkKey& other) const {
    return x == other.x && y == other.y;
  }
};

struct ChunkKeyHash {
  std::size_t operator()(const ChunkKey& k) const {
    return std::hash<int>()(k.x) ^ (std::hash<int>()(k.y) << 1);
  }
};

using ChunkMap = std::unordered_map<ChunkKey, Chunk, ChunkKeyHash>;

void worldToLocal(int worldX, int worldY, ChunkKey& key, int& localX, int& localY);

bool getTileAt(int worldX, int worldY, const ChunkMap& loadedChunks,
               char& outGlyph, short& outColor);

bool getTileType(int worldX, int worldY, const ChunkMap& loadedChunks,
                 unsigned char& outType);

// True if the tile blocks movement (solid wall or closed door)
bool isSolid(int worldX, int worldY, const ChunkMap& loadedChunks);

// True if the tile is water
bool isWater(int worldX, int worldY, const ChunkMap& loadedChunks);

// Toggle door under/near (wx, wy). Returns true if a door was toggled.
bool tryToggleDoor(int worldX, int worldY, ChunkMap& loadedChunks);

bool loadChunkFile(const std::string& filePath, Chunk& chunk);
void loadAllChunks(ChunkMap& loadedChunks,
                   const std::string& basePath = "assets/map/");

void renderMap(WINDOW* win, const ChunkMap& loadedChunks,
               int viewX, int viewY, int viewW, int viewH,
               int camX, int camY);

#endif // !MAP_H
