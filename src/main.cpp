#include "adc_sensor.h"
#include "barometer_sensor.h"
#include "gps_sensor.h"
#include "imu_sensor.h"
#include "logging.h"
#include "rcinput_sensor.h"
#include "telemetry_publisher.h"
#include "utils.h"

#include "main.h"

#include <Common/Util.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

int main(int argc, char *argv[]) {
  logging::log(logging::Level::Info, "Starting sensors_read");
  utils::ProgramOptions options;
  bool show_help = false;
  if (!utils::parse_options(argc, argv, options, show_help)) {
    return EXIT_FAILURE;
  }
  if (show_help) {
    return EXIT_SUCCESS;
  }

  logging::log(logging::Level::Info, "Options: interval=" + std::to_string(options.interval) + "s, rc_channels=" + std::to_string(options.rc_channels) + ", once=" + (options.once ? "true" : "false"));

  if (check_apm()) {
    return EXIT_FAILURE;
  }

  ImuSensor mpu_sensor(ImuType::Mpu9250);
  ImuSensor lsm_sensor(ImuType::Lsm9ds1);
  AdcSensor adc_sensor;
  BarometerSensor barometer_sensor;
  GpsSensor gps_sensor;
  RcInputSensor rc_sensor(options.rc_channels);
  telemetry::TelemetryPublisher publisher;

  if (!publisher.ready()) {
    logging::log(logging::Level::Critical, "Zenoh publisher is not ready. Aborting.");
    return EXIT_FAILURE;
  }
  logging::log(logging::Level::Info, "Zenoh publisher is ready");

  auto publish_or_warn = [&publisher](const std::string &topic,
                                      const std::string &payload) {
    if (!publisher.publish(topic, payload)) {
      logging::log(logging::Level::Warning,
                   std::string("Failed to publish to ") + topic);
    }
  };

  logging::log(logging::Level::Info, "Starting main loop");
  do {
    const std::string timestamp = utils::current_timestamp();
    logging::log(logging::Level::Debug, "Timestamp: " + timestamp);

    const ImuReading mpu_reading = mpu_sensor.read();
    const std::string mpu_payload = format_imu(mpu_sensor.name(), mpu_reading, timestamp);
    logging::log(logging::Level::Debug, "IMU MPU9250 payload: " + mpu_payload);
    publish_or_warn(main_const::imu_topic, mpu_payload);

    const ImuReading lsm_reading = lsm_sensor.read();
    const std::string lsm_payload = format_imu(lsm_sensor.name(), lsm_reading, timestamp);
    logging::log(logging::Level::Debug, "IMU LSM9DS1 payload: " + lsm_payload);
    publish_or_warn(main_const::imu_topic, lsm_payload);

    const std::vector<double> adc_values = adc_sensor.read();
    const std::string adc_payload = format_adc(adc_values, timestamp);
    logging::log(logging::Level::Debug, "ADC payload: " + adc_payload);
    publish_or_warn(main_const::adc_topic, adc_payload);

    const BarometerReading baro_reading = barometer_sensor.read();
    const std::string baro_payload = format_barometer(baro_reading, timestamp);
    logging::log(logging::Level::Debug, "Barometer payload: " + baro_payload);
    publish_or_warn(main_const::barometer_topic, baro_payload);

    const GpsReading gps_reading = gps_sensor.read();
    const std::string gps_payload = format_gps(gps_reading, timestamp);
    logging::log(logging::Level::Debug, "GPS payload: " + gps_payload);
    publish_or_warn(main_const::gps_topic, gps_payload);

    const std::vector<int> rc_values = rc_sensor.read();
    const std::string rc_payload = format_rcinput(rc_values, timestamp);
    logging::log(logging::Level::Debug, "RCInput payload: " + rc_payload);
    publish_or_warn(main_const::rc_topic, rc_payload);

    logging::log(logging::Level::Debug, "Sleeping for " + std::to_string(options.interval) + "s");
    const unsigned int sleep_usecs = static_cast<unsigned int>(options.interval * 1000000.0);
    usleep(sleep_usecs);
  } while (!options.once);

  logging::log(logging::Level::Info, "Main loop finished. Exiting.");
  return EXIT_SUCCESS;
}
