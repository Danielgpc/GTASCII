#ifndef UI_H
#define UI_H

#include "defines.h"
#include "logger.h"
#include <ncurses.h>
#include <string>
#include <vector>

class UI {
public:
  UI();
  ~UI();

  bool init();
  void shutdown();

  // Call on KEY_RESIZE — clears full terminal and repositions windows
  void handleResize();

  // Logging is delegated to the embedded Logger
  Logger& logger() { return log_; }
  const Logger& logger() const { return log_; }

  void log(LogLevel level, const std::string& message, const std::string& tag = "");
  void logInfo(const std::string& msg);
  void logWarn(const std::string& msg);
  void logDebug(const std::string& msg);
  void logSystem(const std::string& msg);
  void logPlayer(const std::string& msg, const std::string& name = "player");

  bool handleChatInput(int key, std::string& outMessage);
  bool isChatActive() const { return chatActive; }
  void setChatActive(bool active);

  void drawInfo(float fps, float px, float py, int chunkX, int chunkY,
                float vx, float vy, float hp, float stamina,
                float maxHp, float maxStamina);
  void drawLog();
  void drawInput();
  void drawBorders();

  WINDOW* gameWin() { return winGame; }
  WINDOW* infoWin() { return winInfo; }
  WINDOW* chatWin() { return winChat; }

  int infoWidth() const { return infoW; }
  int chatHeight() const { return chatH; }
  bool hasInfo() const { return infoW >= INFO_MIN_W && winInfo != nullptr; }
  bool hasChat() const { return chatH >= CHAT_MIN_H && winChat != nullptr; }

  void clearGameView();
  void refreshAll();

private:
  WINDOW* winGame = nullptr;
  WINDOW* winInfo = nullptr;
  WINDOW* winChat = nullptr;

  int infoW = 0;
  int chatH = 0;

  Logger log_;

  static constexpr size_t MAX_CHAT = 200;

  bool chatActive = false;
  std::string chatBuffer;

  void recreateWindows();
  void drawAsciiBox(WINDOW* win, int h, int w);
  void drawAsciiHLine(WINDOW* win, int y, int w);

  int infoTextW() const { return std::max(1, infoW - 2); }
  int chatTextW() const { return std::max(1, getmaxx(winChat) - 2); }

  static std::vector<std::string> wrapText(const std::string& text, int width);

  int levelColor(LogLevel level) const;
};

#endif // !UI_H
