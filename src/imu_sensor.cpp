#include "imu_sensor.h"

#include "logging.h"

#include <Common/InertialSensor.h>
#include <Common/MPU9250.h>
#include <Navio2/LSM9DS1.h>

#include <unistd.h>

#include <cmath>
#include <iomanip>
#include <sstream>

namespace {

constexpr double kPi = 3.14159265358979323846;

std::unique_ptr<InertialSensor> create_mpu() {
  return std::unique_ptr<InertialSensor>(new MPU9250());
}

std::unique_ptr<InertialSensor> create_lsm() {
  return std::unique_ptr<InertialSensor>(new LSM9DS1());
}

} // namespace

ImuSensor::ImuSensor(ImuType type) {
  switch (type) {
  case ImuType::Mpu9250:
    name_ = "MPU9250";
    sensor_ = create_mpu();
    break;
  case ImuType::Lsm9ds1:
    name_ = "LSM9DS1";
    sensor_ = create_lsm();
    break;
  }

  if (!sensor_) {
    logging::log(logging::Level::Error,
                 std::string("Failed to allocate IMU ") + name_);
    return;
  }
  if (!sensor_->probe()) {
    logging::log(logging::Level::Warning,
                 std::string("IMU ") + name_ + " probe failed");
    sensor_.reset();
    return;
  }
  sensor_->initialize();
  logging::log(logging::Level::Info, std::string("Initialized IMU ") + name_);
  usleep(100000);
}

ImuSensor::~ImuSensor() {
  logging::log(logging::Level::Info, std::string("Closing IMU ") + name_);
}

bool ImuSensor::available() const { return static_cast<bool>(sensor_); }

const std::string &ImuSensor::name() const { return name_; }

ImuReading ImuSensor::read() {
  logging::log(logging::Level::Debug, std::string("Reading IMU ") + name_);
  ImuReading result;
  if (!sensor_) {
    logging::log(logging::Level::Warning, std::string("IMU not available: ") + name_);
    return result;
  }
  sensor_->update();
  sensor_->read_accelerometer(&result.ax, &result.ay, &result.az);
  logging::log(logging::Level::Debug, name_ + " Accel: " + std::to_string(result.ax) + " " + std::to_string(result.ay) + " " + std::to_string(result.az));
  sensor_->read_gyroscope(&result.gx_rad, &result.gy_rad, &result.gz_rad);
  logging::log(logging::Level::Debug, name_ + " Gyro: " + std::to_string(result.gx_rad) + " " + std::to_string(result.gy_rad) + " " + std::to_string(result.gz_rad));
  sensor_->read_magnetometer(&result.mx, &result.my, &result.mz);
  logging::log(logging::Level::Debug, name_ + " Mag: " + std::to_string(result.mx) + " " + std::to_string(result.my) + " " + std::to_string(result.mz));
  result.valid = true;
  return result;
}

std::string format_imu(const std::string &name, const ImuReading &data, const std::string &timestamp) {
  if (!data.valid) {
    if (name.empty()) {
      return "IMU: unavailable";
    }
    return std::string("IMU ") + name + ": unavailable";
  }
  std::ostringstream out;
  out << "timestamp=" << timestamp << " name=" << name
      << " ax=" << data.ax << " ay=" << data.ay << " az=" << data.az
      << " gx=" << data.gx_rad << " gy=" << data.gy_rad << " gz=" << data.gz_rad
      << " mx=" << data.mx << " my=" << data.my << " mz=" << data.mz;
  return out.str();
}
