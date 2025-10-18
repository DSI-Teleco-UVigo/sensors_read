#pragma once

#include <memory>
#include <string>

namespace telemetry {

class TelemetryPublisher {
public:
  TelemetryPublisher();
  ~TelemetryPublisher();

  TelemetryPublisher(const TelemetryPublisher &) = delete;
  TelemetryPublisher &operator=(const TelemetryPublisher &) = delete;
  TelemetryPublisher(TelemetryPublisher &&) = delete;
  TelemetryPublisher &operator=(TelemetryPublisher &&) = delete;

  bool ready() const;
  bool publish(const std::string &key_expression, const std::string &message);

private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace telemetry
