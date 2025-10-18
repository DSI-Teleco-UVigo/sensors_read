#pragma once

#include <memory>
#include <string>
#include <vector>

class ADC;

class AdcSensor {
public:
  AdcSensor();
  ~AdcSensor();

  bool available() const;
  std::vector<double> read();

private:
  std::unique_ptr<ADC> adc_;
};

std::string format_adc(const std::vector<double> &values, const std::string &timestamp);
