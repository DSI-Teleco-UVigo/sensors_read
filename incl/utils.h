#pragma once

#include <string>

namespace utils {

struct ProgramOptions {
  double interval = 1.0;
  int rc_channels = 4;
  bool once = false;
};

void print_usage(const char *prog);
bool parse_options(int argc, char *argv[], ProgramOptions &opts,
                   bool &show_help);
std::string current_timestamp();

} // namespace utils
