#pragma once

#include <memory>
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

void print_adc(const std::vector<double> &values);
