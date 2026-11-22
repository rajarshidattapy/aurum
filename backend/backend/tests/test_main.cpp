// Test entry point for suites that exercise DataLoader: it embeds CPython,
// so the interpreter must be running before any test constructs a DataLoader.
#include <gtest/gtest.h>
#include <pybind11/embed.h>
#include <cstdlib>
#include <pybind11/eval.h>

#ifndef AURUM_DATA_SERVICE_PATH
#define AURUM_DATA_SERVICE_PATH "../"
#endif

int main(int argc, char** argv) {
    setenv("AURUM_DATA_SERVICE_PATH", AURUM_DATA_SERVICE_PATH, 1);
    pybind11::scoped_interpreter guard{};
    pybind11::module_::import("sys").attr("path").attr("insert")(0,AURUM_PYTHON_SITE_PACKAGES);
    pybind11::exec(R"(
import sys, types
import pandas as pd
module = types.ModuleType('data_service.fetchers.yahoo_fetcher')
class YahooFetcher:
    def fetch_historical_data(self, *args):
        return pd.DataFrame({'open':[99.,100.,101.], 'high':[101.,102.,103.],
            'low':[98.,99.,100.], 'close':[100.,101.,102.], 'volume':[1000.,1100.,1200.]},
            index=pd.date_range('2025-01-01', periods=3, name='Date'))
module.YahooFetcher = YahooFetcher
sys.modules['data_service.fetchers.yahoo_fetcher'] = module
)");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
