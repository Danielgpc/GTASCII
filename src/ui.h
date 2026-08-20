#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <string>
#include <deque>
#include <vector>
#include "defines.h"

enum class LogLevel {
  DEBUG,
  INFO,
  WARN,
  SYSTEM,
  PLAYER
};

struct LogEntry {
  std::string time;   // HH:MM:SS
  std::string tag;    // DEBUG / INFO / WARN / SYSTEM / username
  std::string message;
  LogLevel level;
};

class UI {
public:
  UI();
  ~UI();

  bool init();
  void shutdown();

  // Call on KEY_RESIZE — clears full terminal and repositions windows
  void handleResize();

  void log(LogLevel level, const std::string& message, const std::string& tag = "");
  void logInfo(const std::string& msg);
  void logWarn(const std::string& msg);
  void logDebug(const std::string& msg);
  void logSystem(const std::string& msg);
  void logPlayer(const std::string& msg, const std::string& name = "player");

  bool handleChatInput(int key, std::string& outMessage);
  bool isChatActive() const { return chatActive; }
  void setChatActive(bool active);

  void drawInfo(float fps, float px, float py, int chunkX, int chunkY, float vx, float vy);
  void drawLog();
  void drawInput();
  void drawBorders();

  WINDOW* gameWin() { return winGame; }
  WINDOW* sideWin() { return winSide; }
  int sideWidth() const { return sideW; }
  bool hasSide() const { return sideW >= SIDE_MIN_W && winSide != nullptr; }

  void clearGameView();
  void refreshAll();

private:
  WINDOW* winGame = nullptr;
  WINDOW* winSide = nullptr;

  int originY = 0;
  int originX = 0;
  int sideW = 0;   // runtime: terminal_width - GAME_W

  std::deque<LogEntry> logEntries;
  static constexpr size_t MAX_LOG = 200;
  static constexpr size_t MAX_CHAT = 200;

  bool chatActive = false;
  std::string chatBuffer;

  void computeOrigin();
  void recreateWindows();
  void drawAsciiBox(WINDOW* win, int h, int w);
  void drawAsciiHLine(WINDOW* win, int y, int w);

  int sideTextW() const { return std::max(1, sideW - 2); }

  static std::vector<std::string> wrapText(const std::string& text, int width);

  std::string nowTimeString() const;
  const char* levelTag(LogLevel level, const std::string& custom) const;
  int levelColor(LogLevel level) const;
};

#endif // !UI_H
