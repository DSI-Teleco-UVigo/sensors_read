#include "barometer_sensor.h"

#include "logging.h"

#include <iomanip>
#include <sstream>
#include <unistd.h>

BarometerSensor::BarometerSensor() {
  logging::log(logging::Level::Info, "Initializing barometer sensor");
  barometer_.initialize();
  if (!barometer_.testConnection()) {
    logging::log(logging::Level::Warning, "Barometer connection test failed");
    return;
  }
  ready_ = true;
  logging::log(logging::Level::Info, "Barometer sensor initialized");
}

BarometerSensor::~BarometerSensor() {
  logging::log(logging::Level::Info, "Closing barometer sensor");
}

bool BarometerSensor::available() const { return ready_; }

BarometerReading BarometerSensor::read() {
  logging::log(logging::Level::Debug, "Reading barometer sensor");
  BarometerReading reading;
  if (!ready_) {
    logging::log(logging::Level::Warning, "Barometer sensor not available");
    return reading;
  }
  barometer_.refreshPressure();
  usleep(10000);
  barometer_.readPressure();
  barometer_.refreshTemperature();
  usleep(10000);
  barometer_.readTemperature();
  barometer_.calculatePressureAndTemperature();
  reading.temperature_c = barometer_.getTemperature();
  reading.pressure_mbar = barometer_.getPressure();
  logging::log(logging::Level::Debug, "Barometer: " + std::to_string(reading.temperature_c) + " C, " + std::to_string(reading.pressure_mbar) + " mbar");
  reading.valid = true;
  return reading;
}

std::string format_barometer(const BarometerReading &reading, const std::string &timestamp) {
  if (!reading.valid) {
    return "Barometer: unavailable";
  }
  std::ostringstream out;
  out << "timestamp=" << timestamp << " temperature=" << reading.temperature_c << " pressure=" << reading.pressure_mbar;
  return out.str();
}
