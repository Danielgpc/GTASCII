#include "ui.h"
#include "defines.h"

#include <ctime>
#include <cstring>
#include <algorithm>
#include <cstdio>

UI::UI() = default;

UI::~UI() {
  shutdown();
}

void UI::computeOrigin() {
  originY = 0;
  originX = 0;
}

void UI::recreateWindows() {
  if (winGame) { delwin(winGame); winGame = nullptr; }
  if (winSide) { delwin(winSide); winSide = nullptr; }

  computeOrigin();

  int termY, termX;
  getmaxyx(stdscr, termY, termX);

  // Game is always fixed 120x40 at top-left
  winGame = newwin(GAME_H, GAME_W, originY, originX);
  if (winGame) keypad(winGame, TRUE);

  // Side panel fills from col 120 to the end of the terminal; height stays SIDE_H
  sideW = termX - GAME_W;
  if (sideW < 0) sideW = 0;

  if (sideW >= SIDE_MIN_W) {
    int sideH = std::min(SIDE_H, termY);
    winSide = newwin(sideH, sideW, originY, originX + GAME_W);
    if (winSide) keypad(winSide, TRUE);
  } else {
    winSide = nullptr;
  }
}

bool UI::init() {
  recreateWindows();
  return winGame != nullptr;
}

void UI::shutdown() {
  if (winGame) { delwin(winGame); winGame = nullptr; }
  if (winSide) { delwin(winSide); winSide = nullptr; }
}

void UI::handleResize() {
  endwin();
  refresh();
  clear();
  refresh();

  recreateWindows();

  int termY, termX;
  getmaxyx(stdscr, termY, termX);
  int frameW = GAME_W + std::max(0, sideW);
  int frameH = std::max(GAME_H, SIDE_H);
  for (int y = 0; y < termY; ++y) {
    for (int x = 0; x < termX; ++x) {
      if (y < frameH && x < frameW) continue;
      mvaddch(y, x, ' ');
    }
  }
  refresh();
}

void UI::drawAsciiBox(WINDOW* win, int h, int w) {
  if (!win) return;
  mvwaddch(win, 0, 0, '+');
  mvwaddch(win, 0, w - 1, '+');
  mvwaddch(win, h - 1, 0, '+');
  mvwaddch(win, h - 1, w - 1, '+');
  for (int x = 1; x < w - 1; ++x) {
    mvwaddch(win, 0, x, '-');
    mvwaddch(win, h - 1, x, '-');
  }
  for (int y = 1; y < h - 1; ++y) {
    mvwaddch(win, y, 0, '|');
    mvwaddch(win, y, w - 1, '|');
  }
}

void UI::drawAsciiHLine(WINDOW* win, int y, int w) {
  if (!win) return;
  mvwaddch(win, y, 0, '+');
  for (int x = 1; x < w - 1; ++x)
    mvwaddch(win, y, x, '-');
  mvwaddch(win, y, w - 1, '+');
}

std::vector<std::string> UI::wrapText(const std::string& text, int width) {
  std::vector<std::string> lines;
  if (width < 1) {
    lines.push_back(text);
    return lines;
  }
  if (text.empty()) {
    lines.push_back("");
    return lines;
  }

  size_t pos = 0;
  while (pos < text.size()) {
    size_t remaining = text.size() - pos;
    if (static_cast<int>(remaining) <= width) {
      lines.push_back(text.substr(pos));
      break;
    }

    size_t end = pos + static_cast<size_t>(width);
    size_t breakAt = text.rfind(' ', end);
    if (breakAt == std::string::npos || breakAt < pos) {
      lines.push_back(text.substr(pos, static_cast<size_t>(width)));
      pos += static_cast<size_t>(width);
    } else {
      lines.push_back(text.substr(pos, breakAt - pos));
      pos = breakAt + 1;
    }
  }
  return lines;
}

std::string UI::nowTimeString() const {
  std::time_t t = std::time(nullptr);
  std::tm* tm = std::localtime(&t);
  char buf[16];
  std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
  return buf;
}

const char* UI::levelTag(LogLevel level, const std::string& custom) const {
  if (!custom.empty()) return custom.c_str();
  switch (level) {
    case LogLevel::DEBUG:  return "DEBUG";
    case LogLevel::INFO:   return "INFO";
    case LogLevel::WARN:   return "WARN";
    case LogLevel::SYSTEM: return "SYSTEM";
    case LogLevel::PLAYER: return "player";
  }
  return "INFO";
}

int UI::levelColor(LogLevel level) const {
  switch (level) {
    case LogLevel::DEBUG:  return 4;
    case LogLevel::INFO:   return 2;
    case LogLevel::WARN:   return 3;
    case LogLevel::SYSTEM: return 1;
    case LogLevel::PLAYER: return 5;
  }
  return 2;
}

void UI::log(LogLevel level, const std::string& message, const std::string& tag) {
  LogEntry e;
  e.time = nowTimeString();
  e.tag = levelTag(level, tag);
  e.message = message;
  e.level = level;
  logEntries.push_back(e);
  while (logEntries.size() > MAX_LOG) {
    logEntries.pop_front();
  }
}

void UI::logInfo(const std::string& msg)   { log(LogLevel::INFO, msg); }
void UI::logWarn(const std::string& msg)   { log(LogLevel::WARN, msg); }
void UI::logDebug(const std::string& msg)  { log(LogLevel::DEBUG, msg); }
void UI::logSystem(const std::string& msg) { log(LogLevel::SYSTEM, msg); }
void UI::logPlayer(const std::string& msg, const std::string& name) {
  log(LogLevel::PLAYER, msg, name);
}

void UI::setChatActive(bool active) {
  chatActive = active;
  if (!active) chatBuffer.clear();
  curs_set(active ? 1 : 0);
}

bool UI::handleChatInput(int key, std::string& outMessage) {
  outMessage.clear();
  if (!chatActive) return false;

  if (key == 27) {
    setChatActive(false);
    return false;
  }
  if (key == '\n' || key == KEY_ENTER || key == '\r') {
    if (!chatBuffer.empty()) {
      outMessage = chatBuffer;
      chatBuffer.clear();
      setChatActive(false);
      return true;
    }
    setChatActive(false);
    return false;
  }
  if (key == KEY_BACKSPACE || key == 127 || key == 8) {
    if (!chatBuffer.empty()) chatBuffer.pop_back();
    return false;
  }
  if (key >= 32 && key <= 126) {
    if (chatBuffer.size() < MAX_CHAT) {
      chatBuffer.push_back(static_cast<char>(key));
    }
  }
  return false;
}

void UI::drawBorders() {
  drawAsciiBox(winGame, GAME_H, GAME_W);

  if (winSide && sideW >= SIDE_MIN_W) {
    int h = std::min(SIDE_H, getmaxy(winSide));
    drawAsciiBox(winSide, h, sideW);
    drawAsciiHLine(winSide, INFO_H - 1, sideW);
    if (h > INPUT_H) {
      drawAsciiHLine(winSide, h - INPUT_H, sideW);
    }
  }
}

void UI::clearGameView() {
  if (!winGame) return;
  for (int y = VIEW_Y; y < VIEW_Y + VIEW_H; ++y) {
    for (int x = VIEW_X; x < VIEW_X + VIEW_W; ++x) {
      mvwaddch(winGame, y, x, ' ');
    }
  }
}

void UI::drawInfo(float fps, float px, float py, int chunkX, int chunkY, float vx, float vy) {
  if (!winSide || sideW < SIDE_MIN_W) return;

  for (int y = 1; y < INFO_H - 1; ++y) {
    for (int x = 1; x < sideW - 1; ++x) {
      mvwaddch(winSide, y, x, ' ');
    }
  }

  wattron(winSide, COLOR_PAIR(1) | A_BOLD);
  mvwprintw(winSide, 1, 2, "GTASCII");
  wattroff(winSide, COLOR_PAIR(1) | A_BOLD);

  mvwprintw(winSide, 2, 2, "FPS  %.1f", fps);
  mvwprintw(winSide, 3, 2, "POS  %.1f  %.1f", px, py);
  mvwprintw(winSide, 4, 2, "CHK  %d,%d", chunkX, chunkY);
  mvwprintw(winSide, 5, 2, "VEL  %.1f  %.1f", vx, vy);
  mvwprintw(winSide, 6, 2, "[t] chat  [q] quit");
}

void UI::drawLog() {
  if (!winSide || sideW < SIDE_MIN_W) return;

  int h = getmaxy(winSide);
  int logTop = INFO_H;
  int logBottom = h - INPUT_H - 1;
  int maxLines = logBottom - logTop + 1;
  if (maxLines < 1) return;

  for (int y = logTop; y <= logBottom; ++y) {
    for (int x = 1; x < sideW - 1; ++x) {
      mvwaddch(winSide, y, x, ' ');
    }
  }

  const int textW = sideTextW();
  std::vector<std::pair<std::string, int>> displayLines;

  for (const auto& e : logEntries) {
    int color = levelColor(e.level);
    char prefix[32];
    std::snprintf(prefix, sizeof(prefix), "%s %-6s ", e.time.c_str(), e.tag.c_str());
    int prefixLen = static_cast<int>(std::strlen(prefix));
    int firstW = textW - prefixLen;
    if (firstW < 8) firstW = textW;

    std::string remaining = e.message;
    std::string firstChunk;
    if (static_cast<int>(remaining.size()) <= firstW) {
      firstChunk = remaining;
      remaining.clear();
    } else {
      size_t end = static_cast<size_t>(firstW);
      size_t br = remaining.rfind(' ', end);
      if (br == std::string::npos || br == 0) {
        firstChunk = remaining.substr(0, end);
        remaining = remaining.substr(end);
      } else {
        firstChunk = remaining.substr(0, br);
        remaining = remaining.substr(br + 1);
      }
    }

    displayLines.push_back({std::string(prefix) + firstChunk, color});

    std::string indent(static_cast<size_t>(std::max(0, prefixLen)), ' ');
    int contW = textW - prefixLen;
    if (contW < 1) contW = textW;

    while (!remaining.empty()) {
      if (static_cast<int>(remaining.size()) <= contW) {
        displayLines.push_back({indent + remaining, color});
        remaining.clear();
      } else {
        size_t end = static_cast<size_t>(contW);
        size_t br = remaining.rfind(' ', end);
        std::string chunk;
        if (br == std::string::npos || br == 0) {
          chunk = remaining.substr(0, end);
          remaining = remaining.substr(end);
        } else {
          chunk = remaining.substr(0, br);
          remaining = remaining.substr(br + 1);
        }
        displayLines.push_back({indent + chunk, color});
      }
    }
  }

  int start = std::max(0, static_cast<int>(displayLines.size()) - maxLines);
  int row = logTop;
  for (size_t i = static_cast<size_t>(start); i < displayLines.size() && row <= logBottom; ++i, ++row) {
    const auto& [text, color] = displayLines[i];
    std::string clipped = text;
    if (static_cast<int>(clipped.size()) > textW) {
      clipped.resize(static_cast<size_t>(textW));
    }
    wattron(winSide, COLOR_PAIR(color));
    mvwprintw(winSide, row, 1, "%s", clipped.c_str());
    wattroff(winSide, COLOR_PAIR(color));
  }
}

void UI::drawInput() {
  if (!winSide || sideW < SIDE_MIN_W) return;

  int h = getmaxy(winSide);
  int inputTop = h - INPUT_H + 1;
  int inputBottom = h - 2;
  int inputRows = inputBottom - inputTop + 1;
  if (inputRows < 1) return;

  for (int y = inputTop; y <= inputBottom; ++y) {
    for (int x = 1; x < sideW - 1; ++x) {
      mvwaddch(winSide, y, x, ' ');
    }
  }

  const int textW = sideTextW();

  if (chatActive) {
    std::string display = "> " + chatBuffer;
    auto lines = wrapText(display, textW);

    int startLine = 0;
    if (static_cast<int>(lines.size()) > inputRows) {
      startLine = static_cast<int>(lines.size()) - inputRows;
    }

    wattron(winSide, COLOR_PAIR(5) | A_BOLD);
    for (int i = 0; i < inputRows && startLine + i < static_cast<int>(lines.size()); ++i) {
      const std::string& line = lines[static_cast<size_t>(startLine + i)];
      mvwprintw(winSide, inputTop + i, 1, "%s", line.c_str());
    }
    wattroff(winSide, COLOR_PAIR(5) | A_BOLD);

    int lastIdx = static_cast<int>(lines.size()) - 1 - startLine;
    if (lastIdx < 0) lastIdx = 0;
    if (lastIdx >= inputRows) lastIdx = inputRows - 1;
    int cursorCol = 1;
    if (!lines.empty()) {
      int vis = startLine + lastIdx;
      if (vis >= 0 && vis < static_cast<int>(lines.size())) {
        cursorCol = 1 + static_cast<int>(lines[static_cast<size_t>(vis)].size());
      }
    }
    if (cursorCol > textW) cursorCol = textW;
    wmove(winSide, inputTop + lastIdx, cursorCol);
  } else {
    wattron(winSide, COLOR_PAIR(4));
    mvwprintw(winSide, inputTop, 1, "> press t to chat");
    wattroff(winSide, COLOR_PAIR(4));
  }
}

void UI::refreshAll() {
  if (winGame) wnoutrefresh(winGame);
  if (winSide) wnoutrefresh(winSide);
  doupdate();
}
