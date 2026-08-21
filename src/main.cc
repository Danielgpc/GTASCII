#include <ncurses.h>
#include <chrono>
#include <thread>
#include <algorithm>
#include <string>
#include <clocale>

#include "defines.h"
#include "player.h"
#include "chunk.h"
#include "ui.h"

int main(int argc, char *argv[]) {
  //> Initialization
  // Force a locale that keeps ACS/UTF-8 from breaking; we use ASCII borders anyway
  setlocale(LC_ALL, "");

  initscr();
  raw();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  nodelay(stdscr, TRUE);
  // Don't let the terminal auto-scroll or leave junk
  clear();
  refresh();

  if (has_colors()) {
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN,   COLOR_BLACK);
    init_pair(2, COLOR_WHITE,   COLOR_BLACK);
    init_pair(3, COLOR_RED,     COLOR_BLACK);
    init_pair(4, COLOR_CYAN,    COLOR_BLACK);
    init_pair(5, COLOR_YELLOW,  COLOR_BLACK);
    init_pair(6, COLOR_BLACK,   COLOR_BLACK);
    init_pair(7, COLOR_BLUE,    COLOR_BLACK);
    init_pair(8, COLOR_MAGENTA, COLOR_BLACK);
  }

  UI ui;
  if (!ui.init()) {
    endwin();
    return 1;
  }

  int termY, termX;
  getmaxyx(stdscr, termY, termX);
  if (termX < GAME_W || termY < GAME_H) {
    // Game needs at least 120x40; chat needs extra width to the right
  }

  ChunkMap loadedChunks;
  loadAllChunks(loadedChunks);

  Player player(10.0f, 10.0f);

  ui.logSystem("World loaded (4x4 chunks, 64x64 tiles)");
  ui.logInfo("Welcome to GTASCII");
  ui.logDebug("Press t to open chat, q to quit");
  //< Initialization

  bool running = true;
  auto lastTime = std::chrono::high_resolution_clock::now();
  float fps = 0.0f;
  float fpsSmooth = 0.0f;

  //> Main loop
  while (running) {
    auto now = std::chrono::high_resolution_clock::now();
    double dt = std::chrono::duration<double>(now - lastTime).count();
    lastTime = now;

    if (dt > 0.0001) {
      float instant = static_cast<float>(1.0 / dt);
      fpsSmooth = fpsSmooth * 0.9f + instant * 0.1f;
      fps = fpsSmooth;
    }

    //> Process input
    int key = getch();

    if (key == KEY_RESIZE) {
      ui.handleResize();
      // skip the rest of this frame's input
    } else if (ui.isChatActive()) {
      std::string msg;
      if (ui.handleChatInput(key, msg)) {
        ui.logPlayer(msg, "player");
      }
    } else {
      if (key == 'q') {
        running = false;
      } else if (key == 't' || key == 'T') {
        ui.setChatActive(true);
        ui.logDebug("Chat opened");
      } else {
        player.handleInput(key);
      }
    }
    //< Process input

    if (!ui.isChatActive()) {
      player.update(static_cast<float>(dt));
      player.clampToWorld(WORLD_WIDTH, WORLD_HEIGHT);
    }

    //> Camera (viewport is fixed VIEW_W x VIEW_H inside the 120x40 frame)
    int camX = static_cast<int>(player.x) - VIEW_W / 2;
    int camY = static_cast<int>(player.y) - VIEW_H / 2;
    camX = std::max(0, std::min(camX, WORLD_WIDTH  - VIEW_W));
    camY = std::max(0, std::min(camY, WORLD_HEIGHT - VIEW_H));
    if (WORLD_WIDTH  < VIEW_W) camX = 0;
    if (WORLD_HEIGHT < VIEW_H) camY = 0;
    //< Camera

    //> Render — always into the fixed 120x40 windows
    ui.clearGameView();
    renderMap(ui.gameWin(), loadedChunks,
              VIEW_X, VIEW_Y, VIEW_W, VIEW_H,
              camX, camY);
    player.render(ui.gameWin(), VIEW_X, VIEW_Y, camX, camY);

    ui.drawBorders();
    ui.drawInfo(fps, player.x, player.y,
                static_cast<int>(player.x) / CHUNK_SIZE,
                static_cast<int>(player.y) / CHUNK_SIZE,
                player.vx, player.vy);
    ui.drawLog();
    ui.drawInput();

    ui.refreshAll();
    //< Render

    auto frameEnd = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(frameEnd - now).count();
    if (elapsed < TARGET_FPS) {
      std::this_thread::sleep_for(std::chrono::duration<double>(TARGET_FPS - elapsed));
    }
  }
  //< Main loop

  ui.shutdown();
  endwin();
  return 0;
}
