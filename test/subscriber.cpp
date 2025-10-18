#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>
#include <cstring>
#include <sstream>
#include <mutex>
#include <vector>
#include <zenoh.hxx>

// A struct to hold the latest sensor readings
struct SensorReadings {
    std::map<std::string, std::string> imu_mpu9250;
    std::map<std::string, std::string> imu_lsm9ds1;
    std::map<std::string, std::string> adc;
    std::map<std::string, std::string> barometer;
    std::map<std::string, std::string> gps;
    std::map<std::string, std::string> rcinput;
};

SensorReadings g_sensor_readings;
std::mutex g_readings_mutex;

void print_pager() {
    std::cout << "\033[2J\033[1;1H"; // Clear screen and move cursor to top-left
    std::cout << "===== Sensor Readings at " <<  std::to_string(static_cast<long long>(std::time(nullptr))) << " =====" << std::endl;
    std::lock_guard<std::mutex> lock(g_readings_mutex);

    auto print_map = [](const std::string& name, const std::map<std::string, std::string>& data) {
        if (data.empty()) {
            return;
        }
        std::cout << name << ": ";
        for (const auto& pair : data) {
            std::cout << pair.first << "=" << pair.second << " ";
        }
        std::cout << std::endl;
    };

    print_map("IMU/MPU9250", g_sensor_readings.imu_mpu9250);
    print_map("IMU/LSM9DS1", g_sensor_readings.imu_lsm9ds1);
    print_map("ADC", g_sensor_readings.adc);
    print_map("Barometer", g_sensor_readings.barometer);
    print_map("GPS", g_sensor_readings.gps);
    print_map("RCInput", g_sensor_readings.rcinput);
}

std::map<std::string, std::string> parse_payload(const std::string& payload) {
    std::map<std::string, std::string> data;
    std::stringstream ss(payload);
    std::string item;
    while (std::getline(ss, item, ' ')) {
        size_t pos = item.find('=');
        if (pos != std::string::npos) {
            data[item.substr(0, pos)] = item.substr(pos + 1);
        }
    }
    return data;
}

void subscriber_callback(const zenoh::Sample& sample) {
    std::string key(sample.get_keyexpr().as_string_view());
    std::string value(sample.get_payload().as_string_view());
    auto data = parse_payload(value);

    std::lock_guard<std::mutex> lock(g_readings_mutex);
    if (key.find("imu") != std::string::npos) {
        if (data["name"] == "MPU9250") {
            g_sensor_readings.imu_mpu9250 = data;
        } else if (data["name"] == "LSM9DS1") {
            g_sensor_readings.imu_lsm9ds1 = data;
        }
    } else if (key.find("adc") != std::string::npos) {
        g_sensor_readings.adc = data;
    } else if (key.find("barometer") != std::string::npos) {
        g_sensor_readings.barometer = data;
    } else if (key.find("gps") != std::string::npos) {
        g_sensor_readings.gps = data;
    } else if (key.find("rcinput") != std::string::npos) {
        g_sensor_readings.rcinput = data;
    }
}

int main() {
    try {
        zenoh::Config config;
        auto session_or_error = zenoh::open(std::move(config));
        if (auto *session = std::get_if<zenoh::Session>(&session_or_error)) {
            std::string keyexpr = "telemetry/sensors/**";
            auto subscriber_or_error = session->declare_subscriber(keyexpr, subscriber_callback);
            if (std::holds_alternative<zenoh::Subscriber>(subscriber_or_error)) {
                while (true) {
                    print_pager();
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            } else {
                std::cerr << "Failed to declare subscriber." << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Failed to open Zenoh session." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
