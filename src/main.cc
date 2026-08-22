#include <algorithm>
#include <chrono>
#include <clocale>
#include <ncurses.h>
#include <sstream>
#include <string>
#include <thread>

#include "defines.h"
#include "map.h"
#include "player.h"
#include "ui.h"

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  setlocale(LC_ALL, "");

  initscr();
  raw();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);
  nodelay(stdscr, TRUE);
  clear();
  refresh();

  if (has_colors()) {
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN,   COLOR_BLACK); // grass / system
    init_pair(2, COLOR_WHITE,   COLOR_BLACK); // roads / info
    init_pair(3, COLOR_RED,     COLOR_BLACK); // walls / warn
    init_pair(4, COLOR_CYAN,    COLOR_BLACK); // water / debug
    init_pair(5, COLOR_YELLOW,  COLOR_BLACK); // doors / player chat
    init_pair(6, COLOR_BLACK,   COLOR_BLACK);
    init_pair(7, COLOR_BLUE,    COLOR_BLACK); // deep water accent
    init_pair(8, COLOR_MAGENTA, COLOR_BLACK); // flowers / decor
  }

  UI ui;
  if (!ui.init()) {
    endwin();
    return 1;
  }

  ChunkMap loadedChunks;
  loadAllChunks(loadedChunks);

  // Spawn on grass near the north-west park
  Player player(20.0f, 20.0f);

  ui.logSystem("World loaded (4x4 chunks, 256x256 tiles)");
  ui.logInfo("Welcome to GTASCII");
  ui.logDebug("t=chat  e=door  q=quit  WASD=move");

  bool running = true;
  auto lastTime = std::chrono::high_resolution_clock::now();
  float fps = 0.0f;
  float fpsSmooth = 0.0f;

  while (running) {
    auto now = std::chrono::high_resolution_clock::now();
    double dt = std::chrono::duration<double>(now - lastTime).count();
    lastTime = now;

    if (dt > 0.0001) {
      float instant = static_cast<float>(1.0 / dt);
      fpsSmooth = fpsSmooth * 0.9f + instant * 0.1f;
      fps = fpsSmooth;
    }

    int key = getch();

    if (key == KEY_RESIZE) {
      ui.handleResize();
    } else if (ui.isChatActive()) {
      std::string msg;
      if (ui.handleChatInput(key, msg)) {
        if (!msg.empty() && msg[0] == '/') {
          std::istringstream iss(msg.substr(1));
          std::string cmd;
          iss >> cmd;
          if (cmd == "hp") {
            float value;
            if (iss >> value) {
              player.hp = std::clamp(value, 0.0f, player.maxHp);
              ui.logSystem("HP set to " + std::to_string(static_cast<int>(player.hp)));
            } else {
              ui.logWarn("Usage: /hp <number>");
            }
          } else if (cmd == "stm" || cmd == "stamina") {
            float value;
            if (iss >> value) {
              player.stamina = std::clamp(value, 0.0f, player.maxStamina);
              ui.logSystem("Stamina set to " + std::to_string(static_cast<int>(player.stamina)));
            } else {
              ui.logWarn("Usage: /stm <number>");
            }
          } else if (cmd == "heal") {
            player.hp = player.maxHp;
            player.stamina = player.maxStamina;
            ui.logSystem("Fully healed");
          } else if (cmd == "tp") {
            float tx, ty;
            if (iss >> tx >> ty) {
              player.x = tx;
              player.y = ty;
              player.vx = player.vy = 0.0f;
              ui.logSystem("Teleported");
            } else {
              ui.logWarn("Usage: /tp <x> <y>");
            }
          } else {
            ui.logWarn("Unknown command: /" + cmd);
          }
        } else {
          ui.logPlayer(msg, "player");
        }
      }
    } else {
      if (key == 'q') {
        running = false;
      } else if (key == 't' || key == 'T') {
        ui.setChatActive(true);
        ui.logDebug("Chat opened");
      } else if (key == 'e' || key == 'E') {
        if (player.interactDoor(loadedChunks)) {
          ui.logInfo("Door toggled");
        }
      } else {
        player.handleInput(key);
      }
    }

    if (!ui.isChatActive()) {
      player.update(static_cast<float>(dt), loadedChunks);
      player.clampToWorld(WORLD_WIDTH, WORLD_HEIGHT);
    }

    int camX = static_cast<int>(player.x) - VIEW_W / 2;
    int camY = static_cast<int>(player.y) - VIEW_H / 2;
    camX = std::max(0, std::min(camX, WORLD_WIDTH - VIEW_W));
    camY = std::max(0, std::min(camY, WORLD_HEIGHT - VIEW_H));
    if (WORLD_WIDTH < VIEW_W) camX = 0;
    if (WORLD_HEIGHT < VIEW_H) camY = 0;

    ui.clearGameView();
    renderMap(ui.gameWin(), loadedChunks, VIEW_X, VIEW_Y, VIEW_W, VIEW_H, camX, camY);
    player.render(ui.gameWin(), VIEW_X, VIEW_Y, camX, camY);

    ui.drawBorders();
    ui.drawInfo(fps, player.x, player.y,
                static_cast<int>(player.x) / CHUNK_SIZE,
                static_cast<int>(player.y) / CHUNK_SIZE,
                player.vx, player.vy,
                player.hp, player.stamina, player.maxHp, player.maxStamina);
    ui.drawLog();
    ui.drawInput();
    ui.refreshAll();

    auto frameEnd = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(frameEnd - now).count();
    if (elapsed < TARGET_FPS) {
      std::this_thread::sleep_for(std::chrono::duration<double>(TARGET_FPS - elapsed));
    }
  }

  ui.shutdown();
  endwin();
  return 0;
}
