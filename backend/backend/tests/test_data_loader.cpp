#include <gtest/gtest.h>
#include "data_loader.hpp"
TEST(DataLoaderTest, LoadMarketData) {
    trading::DataLoader loader;
    auto data = loader.loadMarketData("AAPL");
    EXPECT_EQ(data.symbol,"AAPL"); EXPECT_DOUBLE_EQ(data.last_price,102.0);
    EXPECT_EQ(loader.loadMarketData("AAPL").timestamp, data.timestamp);
}
TEST(DataLoaderTest, LoadHistoricalData) {
    trading::DataLoader loader;
    auto end = std::chrono::system_clock::now();
    auto data = loader.loadHistoricalData("AAPL",end-std::chrono::hours(168),end);
    ASSERT_EQ(data.size(),3u); EXPECT_DOUBLE_EQ(data[0].last_price,100.0);
    EXPECT_LT(data[0].timestamp,data[2].timestamp);
}
