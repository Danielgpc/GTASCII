#include "map.h"

#include <fstream>
#include <sstream>
#include <string>

void worldToLocal(int worldX, int worldY, ChunkKey& key, int& localX, int& localY) {
  key.x = (worldX >= 0) ? (worldX / CHUNK_SIZE)
                        : ((worldX - CHUNK_SIZE + 1) / CHUNK_SIZE);
  key.y = (worldY >= 0) ? (worldY / CHUNK_SIZE)
                        : ((worldY - CHUNK_SIZE + 1) / CHUNK_SIZE);

  localX = (worldX % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
  localY = (worldY % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
}

bool getTileAt(int worldX, int worldY, const ChunkMap& loadedChunks,
               char& outGlyph, short& outColor) {
  ChunkKey key;
  int lx, ly;
  worldToLocal(worldX, worldY, key, lx, ly);

  auto it = loadedChunks.find(key);
  if (it != loadedChunks.end()) {
    outGlyph = it->second.glyphs[ly][lx];
    outColor = it->second.colors[ly][lx];
    return true;
  }
  return false;
}

bool getTileType(int worldX, int worldY, const ChunkMap& loadedChunks,
                 unsigned char& outType) {
  ChunkKey key;
  int lx, ly;
  worldToLocal(worldX, worldY, key, lx, ly);

  auto it = loadedChunks.find(key);
  if (it != loadedChunks.end()) {
    outType = it->second.types[ly][lx];
    return true;
  }
  return false;
}

bool isSolid(int worldX, int worldY, const ChunkMap& loadedChunks) {
  unsigned char t = BLOCK_NORMAL;
  if (!getTileType(worldX, worldY, loadedChunks, t)) return true; // out of world
  return t == BLOCK_SOLID || t == BLOCK_DOOR;
}

bool isWater(int worldX, int worldY, const ChunkMap& loadedChunks) {
  unsigned char t = BLOCK_NORMAL;
  if (!getTileType(worldX, worldY, loadedChunks, t)) return false;
  return t == BLOCK_WATER;
}

bool tryToggleDoor(int worldX, int worldY, ChunkMap& loadedChunks) {
  // Check the tile under the player and the four cardinal neighbours
  const int offsets[5][2] = {{0, 0}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};
  for (const auto& off : offsets) {
    int tx = worldX + off[0];
    int ty = worldY + off[1];
    ChunkKey key;
    int lx, ly;
    worldToLocal(tx, ty, key, lx, ly);
    auto it = loadedChunks.find(key);
    if (it == loadedChunks.end()) continue;

    unsigned char& t = it->second.types[ly][lx];
    char& g = it->second.glyphs[ly][lx];

    if (t == BLOCK_DOOR) {
      t = BLOCK_DOOR_OPEN;
      g = '/'; // open door glyph
      return true;
    }
    if (t == BLOCK_DOOR_OPEN) {
      t = BLOCK_DOOR;
      g = '+'; // closed door glyph
      return true;
    }
  }
  return false;
}

bool loadChunkFile(const std::string& filePath, Chunk& chunk) {
  std::ifstream file(filePath);
  if (!file.is_open()) return false;

  // Defaults
  for (int y = 0; y < CHUNK_SIZE; ++y) {
    for (int x = 0; x < CHUNK_SIZE; ++x) {
      chunk.glyphs[y][x] = '.';
      chunk.colors[y][x] = 1;
      chunk.types[y][x] = BLOCK_NORMAL;
    }
  }

  std::string tag;
  while (file >> tag) {
    if (tag == "GLYPHS") {
      for (int y = 0; y < CHUNK_SIZE; ++y) {
        std::string line;
        file >> line;
        for (int x = 0; x < CHUNK_SIZE && x < static_cast<int>(line.size()); ++x) {
          chunk.glyphs[y][x] = line[x];
        }
      }
    } else if (tag == "COLORS") {
      for (int y = 0; y < CHUNK_SIZE; ++y) {
        std::string line;
        file >> line;
        for (int x = 0; x < CHUNK_SIZE && x < static_cast<int>(line.size()); ++x) {
          chunk.colors[y][x] = static_cast<short>(line[x] - '0');
        }
      }
    } else if (tag == "TYPES") {
      for (int y = 0; y < CHUNK_SIZE; ++y) {
        std::string line;
        file >> line;
        for (int x = 0; x < CHUNK_SIZE && x < static_cast<int>(line.size()); ++x) {
          chunk.types[y][x] = static_cast<unsigned char>(line[x] - '0');
        }
      }
    }
  }
  return true;
}

void loadAllChunks(ChunkMap& loadedChunks, const std::string& basePath) {
  for (int cy = 0; cy < MAP_CHUNKS_Y; ++cy) {
    for (int cx = 0; cx < MAP_CHUNKS_X; ++cx) {
      std::ostringstream path;
      path << basePath << cx << "_" << cy << ".txt";
      Chunk chunk;
      chunk.chunkX = cx;
      chunk.chunkY = cy;
      if (loadChunkFile(path.str(), chunk)) {
        loadedChunks[{cx, cy}] = chunk;
      }
    }
  }
}

void renderMap(WINDOW* win, const ChunkMap& loadedChunks,
               int viewX, int viewY, int viewW, int viewH,
               int camX, int camY) {
  for (int sy = 0; sy < viewH; ++sy) {
    for (int sx = 0; sx < viewW; ++sx) {
      int worldX = sx + camX;
      int worldY = sy + camY;

      char glyph = ' ';
      short color = 0;
      if (getTileAt(worldX, worldY, loadedChunks, glyph, color)) {
        wattron(win, COLOR_PAIR(color));
        mvwaddch(win, viewY + sy, viewX + sx, static_cast<chtype>(glyph));
        wattroff(win, COLOR_PAIR(color));
      } else {
        mvwaddch(win, viewY + sy, viewX + sx, ' ');
      }
    }
  }
}
