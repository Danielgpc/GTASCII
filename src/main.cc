#include <ncurses.h>
#include <chrono>
#include <thread>

#include "defines.h"
#include "player.h"

int main(int argc, char *argv[]) {
  //> Initialization
  initscr(); // Init the screen
  raw(); // Capture all user input (even ctrl-C or ctrl-Z)
  noecho(); // Do not echo user input
  keypad(stdscr, TRUE); // Enable especial keys like functions and arrows
  curs_set(0); // Hide terminal cursor
  nodelay(stdscr, TRUE); // Non-blocking input key polling

  Player player(10.0f, 10.0f);
  //< Initialization

  bool running = true;
  auto lastTime = std::chrono::high_resolution_clock::now();

  //> Main loop
  while (running) {
    // Gat delta time
    auto now = std::chrono::high_resolution_clock::now();
    double dt = std::chrono::duration<double>(now - lastTime).count();
    lastTime = now;

    //> Process input
    int key = getch();
    if (key=='q') running = false;

    player.handleInput(key);
    //< Process input
    
    player.update(dt);
    
    //> Render game
    erase();
    player.render();
    refresh();
    //< Render game

    auto frameEnd = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(frameEnd - now).count();
    if (elapsed < TARGET_FPS) {
      std::this_thread::sleep_for(std::chrono::duration<double>(TARGET_FPS - elapsed));
    }
  }
  //< Main loop

  endwin(); // Finish everything

  return 0;
}
