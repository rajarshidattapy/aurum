#pragma once
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace trading {

// Process-wide logger setup. Call Logger::init() once at startup.
class Logger {
public:
    static void init(const std::string& level = "info") {
        auto logger = spdlog::stdout_color_mt("aurum");
        spdlog::set_default_logger(logger);
        spdlog::set_level(spdlog::level::from_str(level));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    }
};

} // namespace trading
