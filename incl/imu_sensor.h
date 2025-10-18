#pragma once

#include <memory>
#include <string>

struct InertialSensor;

struct ImuReading {
  bool valid = false;
  float ax = 0.0f;
  float ay = 0.0f;
  float az = 0.0f;
  float gx_rad = 0.0f;
  float gy_rad = 0.0f;
  float gz_rad = 0.0f;
  float mx = 0.0f;
  float my = 0.0f;
  float mz = 0.0f;
};

enum class ImuType { Mpu9250, Lsm9ds1 };

class ImuSensor {
public:
  explicit ImuSensor(ImuType type);
  ~ImuSensor();

  bool available() const;
  const std::string &name() const;
  ImuReading read();

private:
  std::string name_;
  std::unique_ptr<InertialSensor> sensor_;
};

std::string format_imu(const std::string &name, const ImuReading &data, const std::string &timestamp);
