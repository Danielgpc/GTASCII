#include "logger.h"

#include <ctime>
#include <cstdio>

std::string Logger::nowTimeString() const {
  std::time_t t = std::time(nullptr);
  std::tm* tm = std::localtime(&t);
  char buf[16];
  std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
  return buf;
}

const char* Logger::levelTag(LogLevel level, const std::string& custom) const {
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

void Logger::log(LogLevel level, const std::string& message, const std::string& tag) {
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

void Logger::logInfo(const std::string& msg)   { log(LogLevel::INFO, msg); }
void Logger::logWarn(const std::string& msg)   { log(LogLevel::WARN, msg); }
void Logger::logDebug(const std::string& msg)  { log(LogLevel::DEBUG, msg); }
void Logger::logSystem(const std::string& msg) { log(LogLevel::SYSTEM, msg); }
void Logger::logPlayer(const std::string& msg, const std::string& name) {
  log(LogLevel::PLAYER, msg, name);
}
