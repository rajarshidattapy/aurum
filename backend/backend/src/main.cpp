#include "data_loader.hpp"
#include "strategy.hpp"
#include "risk_manager.hpp"
#include "order_executor.hpp"
#include "utils/logger.hpp"
#include "common/config.hpp"
#include <csignal>
#include <atomic>
#include <thread>
#include <memory>
#include <map>
#include <pybind11/embed.h>

namespace py = pybind11;

using namespace trading;

namespace {
    std::atomic<bool> running{true};

    void signalHandler(int signal) {
        (void)signal;
        running = false;
    }
}

class TradingEngine {
public:
    TradingEngine() {
        initialize();
    }

    void run() {
        try {
            // 启动订单执行器
            order_executor_->start();

            // 主事件循环
            while (running) {
                settlePaperFills();
                processMarketData();
                processSignals();
                updateRiskMetrics();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // 优雅退出
            shutdown();

        } catch (const std::exception& e) {
            spdlog::critical("Fatal error in trading engine: {}", e.what());
            shutdown();
            throw;
        }
    }

private:
    void initialize() {
        // 初始化日志
        Logger::init();
        spdlog::info("Initializing PAPER-ONLY trading engine. No broker execution.");

        // 加载配置
        config_ = std::make_unique<Config>("config.json");

        // 初始化组件
        data_loader_ = std::make_unique<DataLoader>();
        strategy_ = std::make_unique<MovingAverageStrategy>();
        strategy_->initialize();
        risk_manager_ = std::make_unique<RiskManager>(config_->getRiskLimits());
        order_executor_ = std::make_unique<OrderExecutor>();

        // 设置信号处理
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        spdlog::info("Trading engine initialized successfully");
    }

    void processMarketData() {
        try {
            for (const auto& symbol : config_->getSymbols()) {
                auto market_data = data_loader_->loadMarketData(symbol);
                // onMarketData returns the signals produced by this tick
                auto signals = strategy_->onMarketData(market_data);
                pending_signals_.insert(pending_signals_.end(), signals.begin(), signals.end());
                latest_prices_[symbol] = market_data.last_price;
            }
            risk_manager_->updateCurrentPrices(latest_prices_);
        } catch (const std::exception& e) {
            spdlog::error("Error processing market data: {}", e.what());
        }
    }

    void processSignals() {
        try {
            for (const auto& sig : pending_signals_) {
                auto order = createOrder(sig);
                if (risk_manager_->checkOrderRisk(*order, portfolio_)) {
                    order_executor_->submitOrder(order);
                    pending_orders_.push_back(order);
                }
            }
            pending_signals_.clear();
        } catch (const std::exception& e) {
            spdlog::error("Error processing signals: {}", e.what());
        }
    }

    void updateRiskMetrics() {
        try {
            portfolio_.markToMarket(latest_prices_);
            risk_manager_->updateRiskMetrics(portfolio_);
            auto metrics = risk_manager_->getRiskMetrics();

            // 记录风险指标
            spdlog::debug("Risk metrics - Drawdown: {:.2f}%, Leverage: {:.2f}x",
                metrics["drawdown"] * 100,
                metrics["leverage"]);

        } catch (const std::exception& e) {
            spdlog::error("Error updating risk metrics: {}", e.what());
        }
    }

    void settlePaperFills() {
        for(auto it=pending_orders_.begin();it!=pending_orders_.end();) {
            auto order=*it; auto status=order_executor_->getOrderStatus(order->getOrderId());
            if(status==OrderStatus::FILLED) {
                double qty=order->getQuantity()*(order->getSide()==OrderSide::BUY?1:-1);
                portfolio_.updatePosition(order->getSymbol(),qty,order->getPrice());
                portfolio_.updateCash(-qty*order->getPrice());
                it=pending_orders_.erase(it);
            } else if(status==OrderStatus::CANCELLED || status==OrderStatus::REJECTED) it=pending_orders_.erase(it);
            else ++it;
        }
    }
    void shutdown() {
        spdlog::info("Shutting down trading engine...");
        order_executor_->stop();
        settlePaperFills();
        spdlog::info("Trading engine shutdown complete");
    }

    std::shared_ptr<Order> createOrder(const Strategy::Signal& sig) {
        auto order = std::make_shared<Order>(
            sig.symbol,
            sig.side,
            OrderType::MARKET,
            calculateOrderSize(sig)
        );
        order->setPrice(latest_prices_.at(sig.symbol));
        return order;
    }

    double calculateOrderSize(const Strategy::Signal& sig) {
        // 根据信号强度和资金管理规则计算订单大小
        double portfolio_value = portfolio_.getTotalValue(latest_prices_);
        double price = latest_prices_.count(sig.symbol) ? latest_prices_[sig.symbol] : 0.0;
        if (price <= 0.0) return 0.0;
        if(sig.side==OrderSide::SELL) {
            auto position=portfolio_.getPosition(sig.symbol);
            return position ? position->getQuantity() : 0;
        }
        double notional = portfolio_value * config_->getPositionSizeLimit() * sig.strength;
        return notional / price;  // convert cash amount to units
    }

private:
    std::unique_ptr<Config> config_;
    std::unique_ptr<DataLoader> data_loader_;
    std::unique_ptr<Strategy> strategy_;
    std::unique_ptr<RiskManager> risk_manager_;
    std::unique_ptr<OrderExecutor> order_executor_;
    Portfolio portfolio_;
    std::vector<std::shared_ptr<Order>> pending_orders_;
    std::vector<Strategy::Signal> pending_signals_;
    std::map<std::string, double> latest_prices_;
};

int main() {
    // The DataLoader embeds CPython to reach data_service; the interpreter
    // must be running before any DataLoader is constructed.
    py::scoped_interpreter guard{};
    py::module_::import("sys").attr("path").attr("insert")(0,AURUM_PYTHON_SITE_PACKAGES);
    try {
        TradingEngine engine;
        engine.run();
        return 0;
    } catch (const std::exception& e) {
        spdlog::critical("Fatal error: {}", e.what());
        return 1;
    }
}
