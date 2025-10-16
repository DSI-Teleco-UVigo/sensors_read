#include "gps_sensor.h"

#include "logging.h"

#include <cstdio>
#include <vector>

namespace {

const char *fix_description(int fix_type) {
  switch (fix_type) {
  case 0:
    return "no fix";
  case 1:
    return "dead reckoning";
  case 2:
    return "2D";
  case 3:
    return "3D";
  case 4:
    return "GNSS+DR";
  case 5:
    return "time only";
  default:
    return "unknown";
  }
}

} // namespace

GpsSensor::GpsSensor() {
  gps_ = std::unique_ptr<Ublox>(new Ublox("/dev/spidev0.0"));
  if (!gps_->testConnection()) {
    logging::log(logging::Level::Warning, "GPS test failed");
    gps_.reset();
    return;
  }
  gps_->configureSolutionRate(200);
}

bool GpsSensor::available() const { return static_cast<bool>(gps_); }

GpsReading GpsSensor::read() {
  if (!gps_) {
    return state_;
  }

  std::vector<double> data;
  if (gps_->decodeSingleMessage(Ublox::NAV_POSLLH, data) == 1 &&
      data.size() >= 7) {
    state_.has_position = true;
    state_.time_of_week_s = data[0] / 1000.0;
    state_.longitude_deg = data[1] / 1e7;
    state_.latitude_deg = data[2] / 1e7;
    state_.height_m = data[3] / 1000.0;
    state_.hmsl_m = data[4] / 1000.0;
    state_.horizontal_accuracy_m = data[5] / 1000.0;
    state_.vertical_accuracy_m = data[6] / 1000.0;
  }

  if (gps_->decodeSingleMessage(Ublox::NAV_STATUS, data) == 1 &&
      data.size() >= 2) {
    state_.has_status = true;
    state_.fix_type = static_cast<int>(data[0]);
    state_.fix_ok = (static_cast<int>(data[1]) & 0x01) != 0;
  }

  return state_;
}

void print_gps(const GpsReading &state) {
  if (!state.has_position && !state.has_status) {
    std::printf("GPS: unavailable\n");
    return;
  }

  std::printf("GPS | ");
  if (state.has_position) {
    std::printf("Time: %8.2fs | Lat: %+10.6f | Lon: %+11.6f | Height: %+8.3fm "
                "| HMSL: %+8.3fm | "
                "HAcc: %6.2fm | Vacc: %6.2fm | ",
                state.time_of_week_s, state.latitude_deg, state.longitude_deg,
                state.height_m, state.hmsl_m, state.horizontal_accuracy_m,
                state.vertical_accuracy_m);
  } else {
    std::printf("Position: -- ");
  }

  if (state.has_status) {
    std::printf("Fix: %s (%s)\n", fix_description(state.fix_type),
                state.fix_ok ? "OK" : "NO");
  } else {
    std::printf("Fix: --\n");
  }
}
