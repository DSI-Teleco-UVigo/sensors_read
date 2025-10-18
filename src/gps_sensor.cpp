#include "gps_sensor.h"

#include "logging.h"

#include <iomanip>
#include <sstream>
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
  logging::log(logging::Level::Info, "Initializing GPS sensor");
  gps_ = std::unique_ptr<Ublox>(new Ublox("/dev/spidev0.0"));
  if (!gps_->testConnection()) {
    logging::log(logging::Level::Warning, "GPS test failed");
    gps_.reset();
    return;
  }
  gps_->configureSolutionRate(200);
  logging::log(logging::Level::Info, "GPS sensor initialized");
}

GpsSensor::~GpsSensor() {
  logging::log(logging::Level::Info, "Closing GPS sensor");
}

bool GpsSensor::available() const { return static_cast<bool>(gps_); }

GpsReading GpsSensor::read() {
  logging::log(logging::Level::Debug, "Reading GPS sensor");
  if (!gps_) {
    logging::log(logging::Level::Warning, "GPS sensor not available");
    return state_;
  }

  std::vector<double> data;
  if (gps_->decodeSingleMessage(Ublox::NAV_POSLLH, data) == 1 &&
      data.size() >= 7) {
    logging::log(logging::Level::Debug, "GPS position updated");
    state_.has_position = true;
    state_.time_of_week_s = data[0] / 1000.0;
    state_.longitude_deg = data[1] / 1e7;
    state_.latitude_deg = data[2] / 1e7;
    state_.height_m = data[3] / 1000.0;
    state_.hmsl_m = data[4] / 1000.0;
    state_.horizontal_accuracy_m = data[5] / 1000.0;
    state_.vertical_accuracy_m = data[6] / 1000.0;
    logging::log(logging::Level::Debug, "GPS Pos: " + std::to_string(state_.latitude_deg) + " " + std::to_string(state_.longitude_deg) + " " + std::to_string(state_.height_m));
  }

  if (gps_->decodeSingleMessage(Ublox::NAV_STATUS, data) == 1 &&
      data.size() >= 2) {
    logging::log(logging::Level::Debug, "GPS status updated");
    state_.has_status = true;
    state_.fix_type = static_cast<int>(data[0]);
    state_.fix_ok = (static_cast<int>(data[1]) & 0x01) != 0;
    logging::log(logging::Level::Debug, "GPS Status: " + std::to_string(state_.fix_type) + " " + std::to_string(state_.fix_ok));
  }

  return state_;
}

std::string format_gps(const GpsReading &state, const std::string &timestamp) {
  if (!state.has_position && !state.has_status) {
    return "GPS: unavailable";
  }
  std::ostringstream out;
  out << "timestamp=" << timestamp << " fix_type=" << state.fix_type << " lat=" << state.latitude_deg << " lon=" << state.longitude_deg << " height=" << state.height_m;
  return out.str();
}
