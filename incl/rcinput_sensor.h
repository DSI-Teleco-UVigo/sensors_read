#pragma once

#include <memory>
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

void print_rcinput(const std::vector<int> &values);
