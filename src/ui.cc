#include "ui.h"
#include "defines.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ncurses.h>

UI::UI() = default;

UI::~UI() { shutdown(); }

void UI::recreateWindows() {
  if (winGame) { delwin(winGame); winGame = nullptr; }
  if (winInfo) { delwin(winInfo); winInfo = nullptr; }
  if (winChat) { delwin(winChat); winChat = nullptr; }

  int termY, termX;
  getmaxyx(stdscr, termY, termX);

  winGame = newwin(GAME_H, GAME_W, 0, 0);
  if (winGame) keypad(winGame, TRUE);

  infoW = termX - GAME_W;
  if (infoW < 0) infoW = 0;

  if (infoW >= INFO_MIN_W) {
    winInfo = newwin(GAME_H, infoW, 0, GAME_W);
    if (winInfo) keypad(winInfo, TRUE);
  } else {
    winInfo = nullptr;
  }

  chatH = termY - GAME_H;
  if (chatH < 0) chatH = 0;

  if (chatH >= CHAT_MIN_H) {
    winChat = newwin(chatH, termX, GAME_H, 0);
    if (winChat) keypad(winChat, TRUE);
  } else {
    winChat = nullptr;
  }
}

bool UI::init() {
  recreateWindows();
  return winGame != nullptr;
}

void UI::shutdown() {
  if (winGame) { delwin(winGame); winGame = nullptr; }
  if (winInfo) { delwin(winInfo); winInfo = nullptr; }
  if (winChat) { delwin(winChat); winChat = nullptr; }
}

void UI::handleResize() {
  endwin();
  refresh();
  clear();
  refresh();

  recreateWindows();

  int termY, termX;
  getmaxyx(stdscr, termY, termX);
  for (int y = 0; y < termY; ++y) {
    for (int x = 0; x < termX; ++x) {
      bool insideGame = (y < GAME_H && x < GAME_W);
      bool insideInfo = (winInfo && y < GAME_H && x >= GAME_W && x < GAME_W + infoW);
      bool insideChat = (winChat && y >= GAME_H && y < GAME_H + chatH);
      if (insideGame || insideInfo || insideChat) continue;
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
  log_.log(level, message, tag);
}
void UI::logInfo(const std::string& msg)   { log_.logInfo(msg); }
void UI::logWarn(const std::string& msg)   { log_.logWarn(msg); }
void UI::logDebug(const std::string& msg)  { log_.logDebug(msg); }
void UI::logSystem(const std::string& msg) { log_.logSystem(msg); }
void UI::logPlayer(const std::string& msg, const std::string& name) {
  log_.logPlayer(msg, name);
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

  if (winInfo && infoW >= INFO_MIN_W) {
    drawAsciiBox(winInfo, GAME_H, infoW);
  }

  if (winChat && chatH >= CHAT_MIN_H) {
    int cw = getmaxx(winChat);
    drawAsciiBox(winChat, chatH, cw);
    if (chatH > INPUT_H + 1) {
      drawAsciiHLine(winChat, chatH - INPUT_H - 1, cw);
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

void UI::drawInfo(float fps, float px, float py, int chunkX, int chunkY,
                  float vx, float vy, float hp, float stamina,
                  float maxHp, float maxStamina) {
  if (!winInfo || infoW < INFO_MIN_W) return;

  for (int y = 1; y < GAME_H - 1; ++y) {
    for (int x = 1; x < infoW - 1; ++x) {
      mvwaddch(winInfo, y, x, ' ');
    }
  }

  wattron(winInfo, COLOR_PAIR(1) | A_BOLD);
  mvwprintw(winInfo, 1, 2, "GTASCII");
  wattroff(winInfo, COLOR_PAIR(1) | A_BOLD);

  mvwprintw(winInfo, 3, 2, "FPS  %.1f", fps);
  mvwprintw(winInfo, 4, 2, "POS  %.1f  %.1f", px, py);
  mvwprintw(winInfo, 5, 2, "CHK  %d,%d", chunkX, chunkY);
  mvwprintw(winInfo, 6, 2, "VEL  %.1f  %.1f", vx, vy);

  auto drawBar = [&](int row, const char* label, float current, float max) {
    float ratio = (max > 0.0f) ? (current / max) : 0.0f;
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    int filled = static_cast<int>(ratio * 10.0f + 0.5f);
    if (filled > 10) filled = 10;

    int color;
    if (ratio > 0.6f)      color = 1;
    else if (ratio > 0.3f) color = 5;
    else                   color = 3;

    mvwprintw(winInfo, row, 2, "%-4s ", label);
    mvwaddch(winInfo, row, 7, '[');

    wattron(winInfo, COLOR_PAIR(color));
    for (int i = 0; i < filled; ++i)
      mvwaddch(winInfo, row, 8 + i, '=');
    wattroff(winInfo, COLOR_PAIR(color));

    for (int i = filled; i < 10; ++i)
      mvwaddch(winInfo, row, 8 + i, ' ');

    mvwaddch(winInfo, row, 18, ']');
  };

  drawBar(8, "HP",  hp,      maxHp);
  drawBar(9, "STM", stamina, maxStamina);

  mvwprintw(winInfo, 11, 2, "[t] chat");
  mvwprintw(winInfo, 12, 2, "[e] door");
  mvwprintw(winInfo, 13, 2, "[q] quit");
}

void UI::drawLog() {
  if (!winChat || chatH < CHAT_MIN_H) return;

  int cw = getmaxx(winChat);
  int logTop = 1;
  int logBottom = chatH - INPUT_H - 2;
  int maxLines = logBottom - logTop + 1;
  if (maxLines < 1) return;

  for (int y = logTop; y <= logBottom; ++y) {
    for (int x = 1; x < cw - 1; ++x) {
      mvwaddch(winChat, y, x, ' ');
    }
  }

  const int textW = chatTextW();
  std::vector<std::pair<std::string, int>> displayLines;

  for (const auto& e : log_.entries()) {
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
  for (size_t i = static_cast<size_t>(start);
       i < displayLines.size() && row <= logBottom; ++i, ++row) {
    const auto& [text, color] = displayLines[i];
    std::string clipped = text;
    if (static_cast<int>(clipped.size()) > textW) {
      clipped.resize(static_cast<size_t>(textW));
    }
    wattron(winChat, COLOR_PAIR(color));
    mvwprintw(winChat, row, 1, "%s", clipped.c_str());
    wattroff(winChat, COLOR_PAIR(color));
  }
}

void UI::drawInput() {
  if (!winChat || chatH < CHAT_MIN_H) return;

  int cw = getmaxx(winChat);
  int inputTop = chatH - INPUT_H;
  int inputBottom = chatH - 2;
  int inputRows = inputBottom - inputTop + 1;
  if (inputRows < 1) return;

  for (int y = inputTop; y <= inputBottom; ++y) {
    for (int x = 1; x < cw - 1; ++x) {
      mvwaddch(winChat, y, x, ' ');
    }
  }

  const int textW = chatTextW();

  if (chatActive) {
    std::string display = "> " + chatBuffer;
    auto lines = wrapText(display, textW);

    int startLine = 0;
    if (static_cast<int>(lines.size()) > inputRows) {
      startLine = static_cast<int>(lines.size()) - inputRows;
    }

    wattron(winChat, COLOR_PAIR(5) | A_BOLD);
    for (int i = 0; i < inputRows && startLine + i < static_cast<int>(lines.size()); ++i) {
      const std::string& line = lines[static_cast<size_t>(startLine + i)];
      mvwprintw(winChat, inputTop + i, 1, "%s", line.c_str());
    }
    wattroff(winChat, COLOR_PAIR(5) | A_BOLD);

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
    wmove(winChat, inputTop + lastIdx, cursorCol);
  } else {
    wattron(winChat, COLOR_PAIR(4));
    mvwprintw(winChat, inputTop, 1, "> press t to chat");
    wattroff(winChat, COLOR_PAIR(4));
  }
}

void UI::refreshAll() {
  if (winGame) wnoutrefresh(winGame);
  if (winInfo) wnoutrefresh(winInfo);
  if (winChat) wnoutrefresh(winChat);
  doupdate();
}
