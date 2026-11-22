#include "data_loader.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <cstdlib>
#include <map>

namespace py = pybind11;

namespace trading {

class DataLoader::Impl {
public:
    Impl() {
        try {
            // Locate the Python data_service package. Override with
            // AURUM_DATA_SERVICE_PATH when running from another directory.
            const char* env_path = std::getenv("AURUM_DATA_SERVICE_PATH");
            py::module::import("sys").attr("path").attr("append")(env_path ? env_path : "../");
            data_service_ = py::module::import("data_service.fetchers.yahoo_fetcher");
            fetcher_ = data_service_.attr("YahooFetcher")();

        } catch (const std::exception& e) {
            spdlog::error("Failed to initialize Python interface: {}", e.what());
            throw;
        }
    }

    MarketData loadMarketData(const std::string& symbol) {
        // 60-second cache: the engine loop runs every 100ms and must not
        // hit the network on every iteration.
        const auto now = std::chrono::steady_clock::now();
        auto it = cache_.find(symbol);
        if (it != cache_.end() && now - it->second.second < std::chrono::seconds(60)) {
            return it->second.first;
        }

        auto end = std::chrono::system_clock::now();
        auto start = end - std::chrono::hours(24 * 7);
        auto records = fetchRecords(symbol, start, end);
        if (records.empty()) {
            throw std::runtime_error("No market data returned for " + symbol);
        }
        MarketData data = records.back();
        cache_[symbol] = {data, now};
        return data;
    }

    std::vector<MarketData> loadHistoricalData(
        const std::string& symbol,
        const Timestamp& start,
        const Timestamp& end
    ) {
        return fetchRecords(symbol, start, end);
    }

    void subscribeToRealTimeData(
        const std::string& symbol,
        std::function<void(const MarketData&)> callback
    ) {
        throw std::logic_error("Yahoo polling has no WebSocket subscription; use loadMarketData");
    }

private:
    // Returns one MarketData per OHLCV bar via data_service's YahooFetcher.
    std::vector<MarketData> fetchRecords(
        const std::string& symbol,
        const Timestamp& start,
        const Timestamp& end
    ) {
        try {
            py::object df = fetcher_.attr("fetch_historical_data")(
                symbol,
                py::cast(start),
                py::cast(end)
            );
            // DataFrame -> list of row dicts (iterating a DataFrame would
            // yield column names, not rows).
            py::list records = df.attr("reset_index")().attr("to_dict")("records");

            std::vector<MarketData> result;
            result.reserve(records.size());
            for (const auto& item : records) {
                result.push_back(dictToMarketData(py::cast<py::dict>(item), symbol));
            }
            return result;

        } catch (const std::exception& e) {
            spdlog::error("Failed to load data for {}: {}", symbol, e.what());
            throw;
        }
    }

    static MarketData dictToMarketData(const py::dict& d, const std::string& symbol) {
        MarketData md;
        md.symbol = symbol;
        md.open = d.contains("open") ? d["open"].cast<double>() : 0.0;
        md.high = d.contains("high") ? d["high"].cast<double>() : 0.0;
        md.low = d.contains("low") ? d["low"].cast<double>() : 0.0;
        md.last_price = d.contains("close") ? d["close"].cast<double>() : 0.0;
        md.volume = d.contains("volume") ? d["volume"].cast<double>() : 0.0;
        md.timestamp = std::chrono::system_clock::now();
        for (const char* key : {"date", "Date", "timestamp", "Datetime", "index"}) {
            if (d.contains(key)) {
                try { md.timestamp = d[key].cast<Timestamp>(); } catch (...) {}
                break;
            }
        }
        if (d.contains("indicators")) {
            for (const auto& kv : py::cast<py::dict>(d["indicators"])) {
                md.indicators[kv.first.cast<std::string>()] = kv.second.cast<double>();
            }
        }
        return md;
    }

    py::module_ data_service_;
    py::object fetcher_;
    std::map<std::string, std::pair<MarketData, std::chrono::steady_clock::time_point>> cache_;
};

DataLoader::DataLoader() : pimpl_(std::make_unique<Impl>()) {}
DataLoader::~DataLoader() = default;

MarketData DataLoader::loadMarketData(const std::string& symbol) {
    return pimpl_->loadMarketData(symbol);
}

std::vector<MarketData> DataLoader::loadHistoricalData(
    const std::string& symbol,
    const Timestamp& start,
    const Timestamp& end
) {
    return pimpl_->loadHistoricalData(symbol, start, end);
}

void DataLoader::subscribeToRealTimeData(
    const std::string& symbol,
    std::function<void(const MarketData&)> callback
) {
    pimpl_->subscribeToRealTimeData(symbol, callback);
}

} // namespace trading
