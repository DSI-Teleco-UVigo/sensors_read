#include "barometer_sensor.h"

#include "logging.h"

#include <cstdio>
#include <unistd.h>

BarometerSensor::BarometerSensor() {
  barometer_.initialize();
  if (!barometer_.testConnection()) {
    logging::log(logging::Level::Warning, "Barometer connection test failed");
    return;
  }
  ready_ = true;
}

bool BarometerSensor::available() const { return ready_; }

BarometerReading BarometerSensor::read() {
  BarometerReading reading;
  if (!ready_) {
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
  reading.valid = true;
  return reading;
}

void print_barometer(const BarometerReading &reading) {
  if (!reading.valid) {
    std::printf("Barometer: unavailable\n");
    return;
  }
  std::printf("Barometer | Temp: %6.2f C | Pressure: %8.3f mbar\n",
              reading.temperature_c, reading.pressure_mbar);
}
