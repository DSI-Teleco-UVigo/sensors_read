#include "adc_sensor.h"
#include "barometer_sensor.h"
#include "gps_sensor.h"
#include "imu_sensor.h"
#include "logging.h"
#include "rcinput_sensor.h"
#include "utils.h"

#include <Common/Util.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>

int main(int argc, char *argv[]) {
  utils::ProgramOptions options;
  bool show_help = false;
  if (!utils::parse_options(argc, argv, options, show_help)) {
    return EXIT_FAILURE;
  }
  if (show_help) {
    return EXIT_SUCCESS;
  }

  if (check_apm()) {
    return EXIT_FAILURE;
  }

  ImuSensor mpu_sensor(ImuType::Mpu9250);
  ImuSensor lsm_sensor(ImuType::Lsm9ds1);
  AdcSensor adc_sensor;
  BarometerSensor barometer_sensor;
  GpsSensor gps_sensor;
  RcInputSensor rc_sensor(options.rc_channels);

  const double interval_s = options.interval < 0.1 ? 0.1 : options.interval;

  do {
    std::printf("===== %s =====\n", utils::current_timestamp().c_str());

    print_imu(mpu_sensor.name(), mpu_sensor.read());
    print_imu(lsm_sensor.name(), lsm_sensor.read());
    print_adc(adc_sensor.read());
    print_barometer(barometer_sensor.read());
    print_gps(gps_sensor.read());
    print_rcinput(rc_sensor.read());

    std::fflush(stdout);

    const unsigned int sleep_usecs =
        static_cast<unsigned int>(interval_s * 1000000.0);
    usleep(sleep_usecs);
  } while (options.once);

  return EXIT_SUCCESS;
}
