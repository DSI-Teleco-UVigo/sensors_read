#include "imu_sensor.h"

#include "logging.h"

#include <Common/InertialSensor.h>
#include <Common/MPU9250.h>
#include <Navio2/LSM9DS1.h>

#include <unistd.h>

#include <cmath>
#include <cstdio>

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

ImuSensor::~ImuSensor() = default;

bool ImuSensor::available() const { return static_cast<bool>(sensor_); }

const std::string &ImuSensor::name() const { return name_; }

ImuReading ImuSensor::read() {
  ImuReading result;
  if (!sensor_) {
    return result;
  }
  sensor_->update();
  sensor_->read_accelerometer(&result.ax, &result.ay, &result.az);
  sensor_->read_gyroscope(&result.gx_rad, &result.gy_rad, &result.gz_rad);
  sensor_->read_magnetometer(&result.mx, &result.my, &result.mz);
  result.valid = true;
  return result;
}

void print_imu(const std::string &name, const ImuReading &data) {
  if (!data.valid) {
    if (name.empty()) {
      std::printf("IMU: unavailable\n");
    } else {
      std::printf("IMU %s: unavailable\n", name.c_str());
    }
    return;
  }
  const double gx_deg = data.gx_rad * 180.0 / kPi;
  const double gy_deg = data.gy_rad * 180.0 / kPi;
  const double gz_deg = data.gz_rad * 180.0 / kPi;
  std::printf("IMU %s | Acc: %+7.3f %+7.3f %+7.3f m/s^2 | Gyro: %+8.3f %+8.3f "
              "%+8.3f deg/s | "
              "Mag: %+7.3f %+7.3f %+7.3f uT\n",
              name.empty() ? "IMU" : name.c_str(), data.ax, data.ay, data.az,
              gx_deg, gy_deg, gz_deg, data.mx, data.my, data.mz);
}
