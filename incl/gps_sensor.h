#pragma once

#include <Common/Ublox.h>

#include <limits>
#include <memory>
#include <vector>

struct GpsReading {
  bool has_position = false;
  bool has_status = false;
  double time_of_week_s = std::numeric_limits<double>::quiet_NaN();
  double latitude_deg = std::numeric_limits<double>::quiet_NaN();
  double longitude_deg = std::numeric_limits<double>::quiet_NaN();
  double height_m = std::numeric_limits<double>::quiet_NaN();
  double hmsl_m = std::numeric_limits<double>::quiet_NaN();
  double horizontal_accuracy_m = std::numeric_limits<double>::quiet_NaN();
  double vertical_accuracy_m = std::numeric_limits<double>::quiet_NaN();
  int fix_type = 0;
  bool fix_ok = false;
};

class GpsSensor {
public:
  GpsSensor();

  bool available() const;
  GpsReading read();

private:
  std::unique_ptr<Ublox> gps_;
  GpsReading state_;
};

void print_gps(const GpsReading &state);
