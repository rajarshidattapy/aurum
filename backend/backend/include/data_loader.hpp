#pragma once
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "common/types.hpp"

namespace trading {

// Loads market data from the Python data_service layer (embedded interpreter).
class DataLoader {
public:
    DataLoader();
    ~DataLoader();

    // Latest market data snapshot for a symbol (60s cache).
    MarketData loadMarketData(const std::string& symbol);

    // Historical OHLCV bars between start and end.
    std::vector<MarketData> loadHistoricalData(
        const std::string& symbol,
        const Timestamp& start,
        const Timestamp& end
    );

    // Subscribe to a real-time stream (delegates to the Python fetcher).
    void subscribeToRealTimeData(
        const std::string& symbol,
        std::function<void(const MarketData&)> callback
    );

private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

} // namespace trading
