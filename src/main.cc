#include <ncurses.h>

int main(int argc, char *argv[]) {
  //> Initialization
  // Capture all user input (even ctrl-C or ctrl-Z)
  raw();
  // Do not echo user input
  noecho();
  // Enable especial keys like functions and arrows
  keypad(stdscr, TRUE);
  initscr(); // Init the screen
  //< Initialization

  bool running = true;

  //> Main loop
  while (running) {
    char ch = getch();
    switch (ch) {
    case 'w':
    case KEY_UP:
      break;
    default:
      break;
    }
  }
  //< Main loop

  endwin(); // Finish everything

  return 0;
}
