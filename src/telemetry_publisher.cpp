#include "telemetry_publisher.h"

#include "logging.h"

#include <zenoh.hxx>

#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace telemetry {

namespace {
constexpr const char *kDefaultLogLevel = "error";
} // namespace

class TelemetryPublisher::Impl {
public:
  Impl() {
    logging::log(logging::Level::Info, "Initializing telemetry publisher");
#ifdef ZENOHCXX_ZENOHC
    zenoh::init_logger();
#endif
    auto session_or_error = zenoh::open(z_config_default());
    if (auto *session = std::get_if<zenoh::Session>(&session_or_error)) {
      session_ = std::make_unique<zenoh::Session>(std::move(*session));
      logging::log(logging::Level::Info, "Zenoh session initialized");
    } else {
      logging::log(logging::Level::Critical, "Failed to initialize zenoh session");
    }
  }

  ~Impl() {
      logging::log(logging::Level::Info, "Closing telemetry publisher");
  }

  bool ready() const { return static_cast<bool>(session_); }

  bool publish(const std::string &key, const std::string &message) {
    logging::log(logging::Level::Debug, "Publishing to " + key);
    if (!session_) {
      logging::log(logging::Level::Error, "Cannot publish, no zenoh session");
      return false;
    }
    if (key.empty()) {
      logging::log(logging::Level::Error, "Cannot publish, empty key");
      return false;
    }

    zenoh::Publisher *publisher = find_or_create_publisher(key);
    if (!publisher) {
      logging::log(logging::Level::Error, "Failed to find or create publisher for " + key);
      return false;
    }
    publisher->put(message);
    logging::log(logging::Level::Debug, "Published to " + key);
    return true;
  }

private:
  zenoh::Publisher *find_or_create_publisher(const std::string &key) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = publishers_.find(key);
    if (it != publishers_.end()) {
      return &it->second;
    }

    logging::log(logging::Level::Debug, "Declaring publisher for " + key);
    auto publisher_or_error = session_->declare_publisher(key.c_str());
    if (auto *publisher = std::get_if<zenoh::Publisher>(&publisher_or_error)) {
      auto result = publishers_.emplace(key, std::move(*publisher));
      logging::log(logging::Level::Info, "Declared publisher for " + key);
      return &result.first->second;
    } else {
      logging::log(logging::Level::Error,
                   std::string("Failed to declare publisher for ") + key);
    }
    return nullptr;
  }

  std::unique_ptr<zenoh::Session> session_;
  std::unordered_map<std::string, zenoh::Publisher> publishers_;
  mutable std::mutex mutex_;
};

TelemetryPublisher::TelemetryPublisher() : impl_(new Impl()) {}
TelemetryPublisher::~TelemetryPublisher() = default;

bool TelemetryPublisher::ready() const { return impl_->ready(); }

bool TelemetryPublisher::publish(const std::string &key_expression,
                                 const std::string &message) {
  return impl_->publish(key_expression, message);
}

} // namespace telemetry
