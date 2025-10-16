#include "adc_sensor.h"

#include <Common/Util.h>
#include <Navio2/ADC_Navio2.h>

#include <cmath>
#include <cstdio>
#include <limits>

namespace {
constexpr int kReadFailed = -1;
} // namespace

AdcSensor::AdcSensor() {
  adc_ = std::unique_ptr<ADC>(new ADC_Navio2());

  if (adc_) {
    adc_->initialize();
  }
}

AdcSensor::~AdcSensor() = default;

bool AdcSensor::available() const { return static_cast<bool>(adc_); }

std::vector<double> AdcSensor::read() {
  std::vector<double> values;
  if (!adc_) {
    return values;
  }
  const int channels = adc_->get_channel_count();
  values.reserve(channels);
  for (int idx = 0; idx < channels; ++idx) {
    int raw = adc_->read(idx);
    if (raw == kReadFailed) {
      values.push_back(std::numeric_limits<double>::quiet_NaN());
    } else {
      values.push_back(static_cast<double>(raw) / 1000.0);
    }
  }
  return values;
}

void print_adc(const std::vector<double> &values) {
  if (values.empty()) {
    std::printf("ADC: unavailable\n");
    return;
  }
  std::printf("ADC | ");
  for (std::size_t idx = 0; idx < values.size(); ++idx) {
    if (std::isnan(values[idx])) {
      std::printf("A%zu=--.--V ", idx);
    } else {
      std::printf("A%zu=%6.4fV ", idx, values[idx]);
    }
  }
  std::printf("\n");
}
