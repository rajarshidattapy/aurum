#include "risk_manager.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cmath>

namespace trading {

RiskManager::RiskManager(const RiskLimits& limits) : limits_(limits) {}

bool RiskManager::checkOrderRisk(const Order& order, const Portfolio& portfolio) {
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        // Market orders carry no price; fall back to the latest known price.
        double price = order.getPrice();
        if (price <= 0.0) {
            auto it = current_prices_.find(order.getSymbol());
            if (it != current_prices_.end()) price = it->second;
        }

        if (!std::isfinite(price) || price <= 0 || !std::isfinite(order.getQuantity()) || order.getQuantity() <= 0) return false;
        double position_value = order.getQuantity() * price;
        double portfolio_value = portfolio.getTotalValue(current_prices_);

        if (!std::isfinite(portfolio_value) || portfolio_value <= 0) return false;
        auto held = portfolio.getPosition(order.getSymbol());
        double held_qty = held ? held->getQuantity() : 0;
        if (order.getSide() == OrderSide::SELL) return order.getQuantity() <= held_qty;
        if (position_value > portfolio.getCash()) return false;
        if (portfolio_value > 0.0 && price > 0.0) {
            // 1. Single-position size limit
            if (position_value / portfolio_value > limits_.max_position_size) {
                spdlog::warn("Position size limit exceeded for {}", order.getSymbol());
                return false;
            }

            // 2. Leverage limit
            double total_exposure = portfolio.getTotalExposure() + position_value;
            if (total_exposure / portfolio_value > limits_.max_leverage) {
                spdlog::warn("Leverage limit exceeded");
                return false;
            }

            // 5. Concentration limit
            auto position = portfolio.getPosition(order.getSymbol());
            double new_position_size = (position ? position->getQuantity() : 0) + order.getQuantity();
            double new_concentration = new_position_size * price / portfolio_value;
            if (new_concentration > limits_.position_concentration) {
                spdlog::warn("Position concentration limit exceeded for {}", order.getSymbol());
                return false;
            }
        } else {
            spdlog::debug("Skipping value-based risk checks for {} (no price available)", order.getSymbol());
        }

        // 3. Drawdown limit
        if (portfolio.getDrawdown() > limits_.max_drawdown) {
            spdlog::warn("Drawdown limit exceeded");
            return false;
        }

        // 4. Daily loss limit
        if (portfolio.getDailyPnL() < -limits_.daily_loss_limit) {
            spdlog::warn("Daily loss limit exceeded");
            return false;
        }

        // Refresh cached metrics (mutex already held - do not call the
        // public updateRiskMetrics here, it would deadlock).
        updateRiskMetricsLocked(portfolio);
        return true;

    } catch (const std::exception& e) {
        spdlog::error("Error in risk check: {}", e.what());
        return false;
    }
}

void RiskManager::updateRiskMetrics(const Portfolio& portfolio) {
    std::lock_guard<std::mutex> lock(mutex_);
    updateRiskMetricsLocked(portfolio);
}

void RiskManager::updateRiskMetricsLocked(const Portfolio& portfolio) {
    try {
        current_metrics_["drawdown"] = portfolio.getDrawdown();
        current_metrics_["leverage"] = portfolio.getLeverage();
        current_metrics_["daily_pnl"] = portfolio.getDailyPnL();
        current_metrics_["concentration"] = portfolio.getConcentration();

        spdlog::debug("Risk metrics updated: drawdown={:.2f}%, leverage={:.2f}x, daily_pnl=${:.2f}",
            current_metrics_["drawdown"] * 100,
            current_metrics_["leverage"],
            current_metrics_["daily_pnl"]);

    } catch (const std::exception& e) {
        spdlog::error("Error updating risk metrics: {}", e.what());
    }
}

std::map<std::string, double> RiskManager::getRiskMetrics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_metrics_;
}

void RiskManager::updateCurrentPrices(const std::map<std::string, double>& prices) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [symbol, price] : prices) {
        current_prices_[symbol] = price;
    }
}

} // namespace trading
