#include "chunk.h"

#include <fstream>
#include <string>
#include <sstream>

void worldToLocal(int worldX, int worldY, ChunkKey& key, int& localX, int& localY) {
  key.x = (worldX >= 0) ? (worldX / CHUNK_SIZE) : ((worldX - CHUNK_SIZE + 1) / CHUNK_SIZE);
  key.y = (worldY >= 0) ? (worldY / CHUNK_SIZE) : ((worldY - CHUNK_SIZE + 1) / CHUNK_SIZE);

  localX = (worldX % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
  localY = (worldY % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
}

bool getTileAt(int worldX, int worldY, const ChunkMap& loadedChunks, char& outGlyph, short& outColor) {
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

bool loadChunkFile(const std::string& filePath, Chunk& chunk) {
  std::ifstream file(filePath);
  if (!file.is_open()) return false;

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
        mvwaddch(win, viewY + sy, viewX + sx, glyph);
        wattroff(win, COLOR_PAIR(color));
      } else {
        mvwaddch(win, viewY + sy, viewX + sx, ' ');
      }
    }
  }
}
