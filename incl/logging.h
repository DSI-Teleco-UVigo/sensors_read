#pragma once

#include <string>

namespace logging {

enum class Level { Debug = 0, Info, Warning, Error, Critical };

void set_level(Level level);
Level get_level();
bool set_level(const std::string &name);
void log(Level level, const std::string &message);

} // namespace logging
