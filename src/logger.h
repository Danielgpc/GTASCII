#ifndef LOGGER_H
#define LOGGER_H

#include <deque>
#include <string>

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

class Logger {
public:
  void log(LogLevel level, const std::string& message, const std::string& tag = "");
  void logInfo(const std::string& msg);
  void logWarn(const std::string& msg);
  void logDebug(const std::string& msg);
  void logSystem(const std::string& msg);
  void logPlayer(const std::string& msg, const std::string& name = "player");

  const std::deque<LogEntry>& entries() const { return logEntries; }

private:
  std::deque<LogEntry> logEntries;
  static constexpr size_t MAX_LOG = 200;

  std::string nowTimeString() const;
  const char* levelTag(LogLevel level, const std::string& custom) const;
};

#endif // !LOGGER_H
