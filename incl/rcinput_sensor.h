#pragma once

#include <memory>
#include <string>
#include <vector>

class RCInput;

class RcInputSensor {
public:
  explicit RcInputSensor(int channels);
  ~RcInputSensor();

  bool available() const;
  std::vector<int> read();

private:
  int channels_ = 0;
  std::unique_ptr<RCInput> rc_;
};

std::string format_rcinput(const std::vector<int> &values, const std::string &timestamp);
