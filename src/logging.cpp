#include "logging.h"

#include <atomic>
#include <cctype>
#include <iostream>
#include <mutex>

namespace logging {

namespace {
std::atomic<int> g_level{static_cast<int>(Level::Warning)};
std::mutex g_output_mutex;

const char *to_cstr(Level level) {
  switch (level) {
  case Level::Debug:
    return "DEBUG";
  case Level::Info:
    return "INFO";
  case Level::Warning:
    return "WARNING";
  case Level::Error:
    return "ERROR";
  case Level::Critical:
    return "CRITICAL";
  default:
    return "LOG";
  }
}

Level parse_level_name(const std::string &name, bool &ok) {
  std::string upper;
  upper.reserve(name.size());
  for (char ch : name) {
    upper.push_back(
        static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
  }
  if (upper == "DEBUG") {
    ok = true;
    return Level::Debug;
  }
  if (upper == "INFO") {
    ok = true;
    return Level::Info;
  }
  if (upper == "WARNING") {
    ok = true;
    return Level::Warning;
  }
  if (upper == "ERROR") {
    ok = true;
    return Level::Error;
  }
  if (upper == "CRITICAL") {
    ok = true;
    return Level::Critical;
  }
  ok = false;
  return Level::Warning;
}

} // namespace

void set_level(Level level) {
  g_level.store(static_cast<int>(level), std::memory_order_relaxed);
}

Level get_level() {
  return static_cast<Level>(g_level.load(std::memory_order_relaxed));
}

bool set_level(const std::string &name) {
  bool ok = false;
  Level level = parse_level_name(name, ok);
  if (ok) {
    set_level(level);
  }
  return ok;
}

void log(Level level, const std::string &message) {
  if (static_cast<int>(level) < g_level.load(std::memory_order_relaxed)) {
    return;
  }
  std::lock_guard<std::mutex> lock(g_output_mutex);
  std::cerr << '[' << to_cstr(level) << "] " << message << '\n';
}

} // namespace logging
