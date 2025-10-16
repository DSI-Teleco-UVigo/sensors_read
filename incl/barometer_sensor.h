#pragma once

#include <Common/MS5611.h>

#include <limits>

struct BarometerReading {
  bool valid = false;
  double temperature_c = std::numeric_limits<double>::quiet_NaN();
  double pressure_mbar = std::numeric_limits<double>::quiet_NaN();
};

class BarometerSensor {
public:
  BarometerSensor();

  bool available() const;
  BarometerReading read();

private:
  bool ready_ = false;
  MS5611 barometer_;
};

void print_barometer(const BarometerReading &reading);
