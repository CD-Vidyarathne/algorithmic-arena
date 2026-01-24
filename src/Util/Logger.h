#pragma once

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

class Logger {
public:
  static std::shared_ptr<spdlog::logger> &get() {
    static std::shared_ptr<spdlog::logger> instance = nullptr;
    if (!instance) {
      instance = spdlog::stdout_color_mt("console");
      instance->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
      instance->set_level(spdlog::level::info); // default log level
    }
    return instance;
  }
};
