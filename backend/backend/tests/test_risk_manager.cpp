#include <gtest/gtest.h>
#include "risk_manager.hpp"

class RiskManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        trading::RiskManager::RiskLimits limits{
            .max_position_size = 0.1,
            .max_drawdown = 0.2,
            .max_leverage = 2.0,
            .daily_loss_limit = 10000.0,
            .position_concentration = 0.3
        };
        risk_manager_ = std::make_unique<trading::RiskManager>(limits);
    }

    std::unique_ptr<trading::RiskManager> risk_manager_;
};

TEST_F(RiskManagerTest, CheckOrderRisk) {
    trading::Order order("AAPL", trading::OrderSide::BUY,
                        trading::OrderType::MARKET, 100);
    trading::Portfolio portfolio;

    EXPECT_FALSE(risk_manager_->checkOrderRisk(order, portfolio));
    risk_manager_->updateCurrentPrices({{"AAPL",150.0}});
    EXPECT_TRUE(risk_manager_->checkOrderRisk(order, portfolio));
}
TEST(RiskBoundary, RejectInvalidAndOversize) {
    trading::RiskManager risk({0.1,0.2,2.0,10000.,0.3});
    risk.updateCurrentPrices({{"AAPL",150.0}});
    trading::Portfolio p;
    trading::Order zero("AAPL",trading::OrderSide::BUY,trading::OrderType::MARKET,0);
    EXPECT_FALSE(risk.checkOrderRisk(zero,p));
    trading::Order large("AAPL",trading::OrderSide::BUY,trading::OrderType::MARKET,1000);
    EXPECT_FALSE(risk.checkOrderRisk(large,p));
    trading::Order sell("AAPL",trading::OrderSide::SELL,trading::OrderType::MARKET,1);
    EXPECT_FALSE(risk.checkOrderRisk(sell,p));
}
