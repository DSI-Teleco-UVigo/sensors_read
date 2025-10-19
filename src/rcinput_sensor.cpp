#include "rcinput_sensor.h"

#include "logging.h"

#include <Common/Util.h>
#include <Navio2/RCInput_Navio2.h>

#include <array>
#include <cmath>
#include <sstream>

namespace {
constexpr int kReadFailed = -1;
constexpr double kPwmMin = 1000.0;
constexpr double kPwmMax = 2000.0;
constexpr double kPwmRange = kPwmMax - kPwmMin;

struct AxisMapping {
  const char *name;
  std::size_t channel;
};

constexpr std::array<AxisMapping, 4> kAxes = {{
    {"roll", 0},
    {"pitch", 1},
    {"throttle", 2},
    {"yaw", 3},
}};

int normalize_pwm(int raw_value) {
  if (raw_value <= 0) {
    return 0;
  }
  if (raw_value <= kPwmMin) {
    return 0;
  }
  if (raw_value >= kPwmMax) {
    return 100;
  }
  const double normalized = ((static_cast<double>(raw_value) - kPwmMin) / kPwmRange) * 100.0;
  return static_cast<int>(std::lround(normalized));
}

int normalized_axis_value(const std::vector<int> &values, std::size_t channel) {
  if (channel >= values.size()) {
    logging::log(logging::Level::Warning,
                 "RCInput channel " + std::to_string(channel) + " not available for normalization");
    return 0;
  }
  return normalize_pwm(values[channel]);
}
} // namespace

#include "logging.h"

RcInputSensor::RcInputSensor(int channels)
    : channels_(channels > 0 ? channels : 0) {
  logging::log(logging::Level::Info, "Initializing RCInput sensor");
  rc_ = std::unique_ptr<RCInput>(new RCInput_Navio2());

  if (rc_) {
    rc_->initialize();
    logging::log(logging::Level::Info, "RCInput sensor initialized");
  } else {
    logging::log(logging::Level::Error, "Failed to initialize RCInput sensor");
  }
}

RcInputSensor::~RcInputSensor() {
  logging::log(logging::Level::Info, "Closing RCInput sensor");
}

bool RcInputSensor::available() const { return rc_ && channels_ > 0; }

std::vector<int> RcInputSensor::read() {
  logging::log(logging::Level::Debug, "Reading RCInput sensor");
  std::vector<int> values;
  if (!available()) {
    logging::log(logging::Level::Warning, "RCInput sensor not available");
    return values;
  }
  values.reserve(channels_);
  for (int idx = 0; idx < channels_; ++idx) {
    int value = rc_->read(idx);
    if (value == kReadFailed || value <= 0) {
      logging::log(logging::Level::Warning, "Failed to read RCInput channel " + std::to_string(idx));
      values.push_back(-1);
    } else {
      logging::log(logging::Level::Debug, "RCInput channel " + std::to_string(idx) + ": " + std::to_string(value));
      values.push_back(value);
    }
  }
  return values;
}

std::string format_rcinput(const std::vector<int> &values, const std::string &timestamp) {
  if (values.empty()) {
    return "RC Input: unavailable";
  }
  std::ostringstream out;
  out << "timestamp=" << timestamp;
  for (const auto &axis : kAxes) {
    const int normalized_value = normalized_axis_value(values, axis.channel);
    out << " " << axis.name << "=" << normalized_value;
  }
  return out.str();
}
