#include "rcinput_sensor.h"

#include <Common/Util.h>
#include <Navio2/RCInput_Navio2.h>

#include <cstdio>

namespace {
constexpr int kReadFailed = -1;
} // namespace

RcInputSensor::RcInputSensor(int channels)
    : channels_(channels > 0 ? channels : 0) {
  rc_ = std::unique_ptr<RCInput>(new RCInput_Navio2());

  if (rc_) {
    rc_->initialize();
  }
}

RcInputSensor::~RcInputSensor() = default;

bool RcInputSensor::available() const { return rc_ && channels_ > 0; }

std::vector<int> RcInputSensor::read() {
  std::vector<int> values;
  if (!available()) {
    return values;
  }
  values.reserve(channels_);
  for (int idx = 0; idx < channels_; ++idx) {
    int value = rc_->read(idx);
    if (value == kReadFailed || value <= 0) {
      values.push_back(-1);
    } else {
      values.push_back(value);
    }
  }
  return values;
}

void print_rcinput(const std::vector<int> &values) {
  if (values.empty()) {
    std::printf("RC Input: unavailable\n");
    return;
  }
  std::printf("RC Input | ");
  for (std::size_t idx = 0; idx < values.size(); ++idx) {
    if (values[idx] < 0) {
      std::printf("CH%zu=---- ", idx);
    } else {
      std::printf("CH%zu=%4d ", idx, values[idx]);
    }
  }
  std::printf("\n");
}
