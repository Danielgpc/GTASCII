#ifndef CHUNK_H
#define CHUNK_H

#include <array>
#include <unordered_map>
#include <cstddef>
#include <functional>
#include <string>
#include <ncurses.h>

constexpr int CHUNK_SIZE = 16;
constexpr int MAP_CHUNKS_X = 4;
constexpr int MAP_CHUNKS_Y = 4;
constexpr int WORLD_WIDTH  = MAP_CHUNKS_X * CHUNK_SIZE; // 64
constexpr int WORLD_HEIGHT = MAP_CHUNKS_Y * CHUNK_SIZE; // 64

struct Chunk {
  int chunkX = 0;
  int chunkY = 0;

  std::array<std::array<char, CHUNK_SIZE>, CHUNK_SIZE> glyphs{};
  std::array<std::array<short, CHUNK_SIZE>, CHUNK_SIZE> colors{};
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
bool getTileAt(int worldX, int worldY, const ChunkMap& loadedChunks, char& outGlyph, short& outColor);
bool loadChunkFile(const std::string& filePath, Chunk& chunk);
void loadAllChunks(ChunkMap& loadedChunks, const std::string& basePath = "assets/map/");
void renderMap(WINDOW* win, const ChunkMap& loadedChunks,
               int viewX, int viewY, int viewW, int viewH,
               int camX, int camY);

#endif
