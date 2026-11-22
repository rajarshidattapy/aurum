#include "strategy.hpp"
#include <numeric>
#include <stdexcept>
#include <cmath>
namespace trading {
MovingAverageStrategy::MovingAverageStrategy(int short_period, int long_period)
    : short_period_(short_period), long_period_(long_period) {
    if (short_period <= 0 || long_period <= short_period) throw std::invalid_argument("Require 0 < short < long");
}
void MovingAverageStrategy::initialize() { prices_.clear(); last_seen_.clear(); previous_regime_.clear(); }
std::vector<Strategy::Signal> MovingAverageStrategy::onMarketData(const MarketData& data) {
    if (!std::isfinite(data.last_price) || data.last_price <= 0) return {};
    if (last_seen_.count(data.symbol) && data.timestamp <= last_seen_[data.symbol]) return {};
    last_seen_[data.symbol] = data.timestamp;
    auto& prices = prices_[data.symbol];
    prices.push_back(data.last_price);
    if (prices.size() > static_cast<size_t>(long_period_)) prices.pop_front();
    if (prices.size() < static_cast<size_t>(long_period_)) return {};
    double short_ma = std::accumulate(prices.end()-short_period_, prices.end(), 0.0)/short_period_;
    double long_ma = std::accumulate(prices.begin(), prices.end(), 0.0)/long_period_;
    int regime = short_ma > long_ma ? 1 : short_ma < long_ma ? -1 : 0;
    int previous = previous_regime_[data.symbol];
    if (regime != 0) previous_regime_[data.symbol] = regime;
    if (regime == 0 || previous == 0 || regime == previous) return {};
    return {{data.symbol, regime > 0 ? OrderSide::BUY : OrderSide::SELL,
             std::min(1.0, std::abs(short_ma-long_ma)/long_ma), data.timestamp}};
}
void MovingAverageStrategy::onOrderUpdate(const Order&) {}
}
