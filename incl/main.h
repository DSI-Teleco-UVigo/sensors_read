#pragma once

#include <string>

namespace main_const {

const std::string base_topic = "telemetry/sensors";
const std::string header_topic = base_topic + "/header";
const std::string imu_topic = base_topic + "/imu";
const std::string adc_topic = base_topic + "/adc";
const std::string barometer_topic = base_topic + "/barometer";
const std::string gps_topic = base_topic + "/gps";
const std::string rc_topic = base_topic + "/rcinput";

} // namespace main_const
