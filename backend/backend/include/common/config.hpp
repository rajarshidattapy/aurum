#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "risk_manager.hpp"

namespace trading {

// Runtime configuration for the trading engine.
// Reads config.json when present; falls back to sane defaults so the
// engine runs out of the box.
class Config {
public:
    explicit Config(const std::string& path) { load(path); }

    std::vector<std::string> getSymbols() const { return symbols_; }
    RiskManager::RiskLimits getRiskLimits() const { return risk_limits_; }
    double getPositionSizeLimit() const { return position_size_limit_; }

private:
    void load(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            spdlog::warn("Config file {} not found, using defaults", path);
            return;
        }
        try {
            nlohmann::json j;
            file >> j;
            if (j.contains("symbols")) symbols_ = j["symbols"].get<std::vector<std::string>>();
            if (j.contains("position_size_limit")) position_size_limit_ = j["position_size_limit"].get<double>();
            if (j.contains("risk_limits")) {
                const auto& rl = j["risk_limits"];
                if (rl.contains("max_position_size")) risk_limits_.max_position_size = rl["max_position_size"].get<double>();
                if (rl.contains("max_drawdown")) risk_limits_.max_drawdown = rl["max_drawdown"].get<double>();
                if (rl.contains("max_leverage")) risk_limits_.max_leverage = rl["max_leverage"].get<double>();
                if (rl.contains("daily_loss_limit")) risk_limits_.daily_loss_limit = rl["daily_loss_limit"].get<double>();
                if (rl.contains("position_concentration")) risk_limits_.position_concentration = rl["position_concentration"].get<double>();
            }
            if (symbols_.empty() || !std::isfinite(position_size_limit_) || position_size_limit_<=0 || position_size_limit_>1 ||
                !std::isfinite(risk_limits_.max_position_size) || risk_limits_.max_position_size<=0 || risk_limits_.max_position_size>1 ||
                !std::isfinite(risk_limits_.max_drawdown) || risk_limits_.max_drawdown<0 || risk_limits_.max_drawdown>1 ||
                !std::isfinite(risk_limits_.max_leverage) || risk_limits_.max_leverage<=0 ||
                !std::isfinite(risk_limits_.daily_loss_limit) || risk_limits_.daily_loss_limit<0 ||
                !std::isfinite(risk_limits_.position_concentration) || risk_limits_.position_concentration<=0 || risk_limits_.position_concentration>1)
                throw std::invalid_argument("Invalid paper-engine risk configuration");
            spdlog::info("Loaded config from {} ({} symbols)", path, symbols_.size());
        } catch (const std::exception& e) {
            throw std::runtime_error("Invalid config file " + path + ": " + e.what());
        }
    }

    std::vector<std::string> symbols_ = {"AAPL", "MSFT"};
    RiskManager::RiskLimits risk_limits_{0.1, 0.2, 2.0, 10000.0, 0.3};
    double position_size_limit_ = 0.1;
};

} // namespace trading
