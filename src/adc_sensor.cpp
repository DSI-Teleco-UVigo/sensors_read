#include "adc_sensor.h"

#include <Common/Util.h>
#include <Navio2/ADC_Navio2.h>

#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

namespace {
constexpr int kReadFailed = -1;
} // namespace

#include "logging.h"

AdcSensor::AdcSensor() {
  logging::log(logging::Level::Info, "Initializing ADC sensor");
  adc_ = std::unique_ptr<ADC>(new ADC_Navio2());

  if (adc_) {
    adc_->initialize();
    logging::log(logging::Level::Info, "ADC sensor initialized");
  } else {
    logging::log(logging::Level::Error, "Failed to initialize ADC sensor");
  }
}

AdcSensor::~AdcSensor() {
  logging::log(logging::Level::Info, "Closing ADC sensor");
}

bool AdcSensor::available() const { return static_cast<bool>(adc_); }

std::vector<double> AdcSensor::read() {
  logging::log(logging::Level::Debug, "Reading ADC sensor");
  std::vector<double> values;
  if (!adc_) {
    logging::log(logging::Level::Warning, "ADC sensor not available");
    return values;
  }
  const int channels = adc_->get_channel_count();
  values.reserve(channels);
  for (int idx = 0; idx < channels; ++idx) {
    int raw = adc_->read(idx);
    if (raw == kReadFailed) {
      logging::log(logging::Level::Warning, "Failed to read ADC channel " + std::to_string(idx));
      values.push_back(std::numeric_limits<double>::quiet_NaN());
    } else {
      double value = static_cast<double>(raw) / 1000.0;
      logging::log(logging::Level::Debug, "ADC channel " + std::to_string(idx) + ": " + std::to_string(value));
      values.push_back(value);
    }
  }
  return values;
}

std::string format_adc(const std::vector<double> &values, const std::string &timestamp) {
  if (values.empty()) {
    return "ADC: unavailable";
  }
  std::ostringstream out;
  out << "timestamp=" << timestamp;
  for (std::size_t idx = 0; idx < values.size(); ++idx) {
    out << " a" << idx << "=" << values[idx];
  }
  return out.str();
}
