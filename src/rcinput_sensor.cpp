#include "rcinput_sensor.h"

#include <Common/Util.h>
#include <Navio2/RCInput_Navio2.h>

#include <iomanip>
#include <sstream>

namespace {
constexpr int kReadFailed = -1;
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
  for (std::size_t idx = 0; idx < values.size(); ++idx) {
    out << " c" << idx << "=" << values[idx];
  }
  return out.str();
}
