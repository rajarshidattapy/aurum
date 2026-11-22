#include <gtest/gtest.h>
#include "strategy.hpp"
#include "order_executor.hpp"
#include <thread>
using namespace trading;
TEST(PositionAccounting, PartialSellKeepsCostBasis) {
    Position p("A");p.updatePosition(10,100);p.updatePosition(-4,150);
    EXPECT_DOUBLE_EQ(p.getAveragePrice(),100);EXPECT_DOUBLE_EQ(p.getQuantity(),6);
    EXPECT_THROW(p.updatePosition(-7,100),std::invalid_argument);
}
TEST(Strategy, RejectsInvalidWindows) {EXPECT_THROW(MovingAverageStrategy(10,5),std::invalid_argument);}
TEST(Strategy, DuplicateBarsAndSymbolsAreIndependent) {
    MovingAverageStrategy s(2,3); MarketData data{};data.symbol="A";data.last_price=100;
    data.timestamp=Timestamp{}+std::chrono::seconds(1);
    EXPECT_TRUE(s.onMarketData(data).empty());EXPECT_TRUE(s.onMarketData(data).empty());
    data.symbol="B";data.last_price=200;EXPECT_TRUE(s.onMarketData(data).empty());
}
TEST(PaperExecutor, StartTwiceFillAndStatus) {
    OrderExecutor e;e.start();e.start();auto o=std::make_shared<Order>("A",OrderSide::BUY,OrderType::MARKET,1);
    o->setPrice(100);e.submitOrder(o);
    for(int i=0;i<100 && e.getOrderStatus(o->getOrderId())==OrderStatus::PENDING;++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    EXPECT_EQ(e.getOrderStatus(o->getOrderId()),OrderStatus::FILLED);
    EXPECT_THROW(e.submitOrder(o),std::invalid_argument);e.stop();e.stop();
}
