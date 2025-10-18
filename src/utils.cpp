#include "utils.h"

#include "logging.h"

#include <cstdlib>
#include <ctime>
#include <getopt.h>
#include <iostream>
#include <string>
#include <unistd.h>

namespace utils {

void print_usage(const char *prog) {
  std::cout << "Usage: " << prog << " [options]\n"
            << "  --interval <seconds>     Sampling interval (default: 1.0)\n"
            << "  --rc-channels <count>    Number of RC channels (default: 4)\n"
            << "  --once                   Read sensors only once\n"

            << "  --log-level <level>      Log verbosity "
               "(DEBUG/INFO/WARNING/ERROR/CRITICAL)\n"
            << "  --help                   Show this message\n";
}

bool parse_options(int argc, char *argv[], ProgramOptions &opts,
                   bool &show_help) {
  logging::log(logging::Level::Debug, "Parsing options");
  show_help = false;
  const char *short_opts = "t:c:ol:h";
  const struct option long_opts[] = {
      {"interval", required_argument, nullptr, 't'},
      {"rc-channels", required_argument, nullptr, 'c'},
      {"once", no_argument, nullptr, 'o'},
      {"log-level", required_argument, nullptr, 'l'},
      {"help", no_argument, nullptr, 'h'},
      {nullptr, 0, nullptr, 0}};

  int option_index = 0;
  int opt;
  while ((opt = getopt_long(argc, argv, short_opts, long_opts,
                            &option_index)) != -1) {
    switch (opt) {
    case 't': {
      if (!optarg) {
        logging::log(logging::Level::Error, "Missing argument for --interval");
        return false;
      }
      char *end = nullptr;
      double value = std::strtod(optarg, &end);
      if (!end || *end != '\0' || value < 0.0) {
        logging::log(logging::Level::Error, "Invalid interval value");
        return false;
      }
      opts.interval = value;
      logging::log(logging::Level::Debug, "Interval set to " + std::to_string(value));
      break;
    }

    case 'c': {
      if (!optarg) {
        logging::log(logging::Level::Error,
                     "Missing argument for --rc-channels");
        return false;
      }
      char *end = nullptr;
      long value = std::strtol(optarg, &end, 10);
      if (!end || *end != '\0' || value <= 0 || value > 14) {
        logging::log(logging::Level::Error, "Invalid RC channel count");
        return false;
      }
      opts.rc_channels = static_cast<int>(value);
      logging::log(logging::Level::Debug, "RC channels set to " + std::to_string(value));
      break;
    }

    case 'o':
      opts.once = true;
      logging::log(logging::Level::Debug, "Once option set to true");
      break;

    case 'l':
      if (!optarg || !logging::set_level(optarg)) {
        logging::log(logging::Level::Error, "Invalid log level");
        return false;
      }
      logging::log(logging::Level::Debug, "Log level set");
      break;

    case 'h':
      print_usage(argv[0]);
      show_help = true;
      return true;

    default:
      print_usage(argv[0]);
      return false;
    }
  }

  logging::log(logging::Level::Debug, "Finished parsing options");
  return true;
}

std::string current_timestamp() {
  logging::log(logging::Level::Debug, "Getting current timestamp");
  std::time_t now = std::time(nullptr);
  std::string timestamp = std::to_string(static_cast<long long>(now));
  logging::log(logging::Level::Debug, "Timestamp: " + timestamp);
  return timestamp;
}

} // namespace utils
