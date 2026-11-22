#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <cmath>
#include "strategy.hpp"
#include "risk_manager.hpp"
namespace py = pybind11;
using namespace trading;
PYBIND11_MODULE(aurum_core,m) {
    m.doc() = "Aurum C++ signal generation and long-only paper risk checks";
    m.def("signals",[](const std::vector<double>& prices,int short_window,int long_window){
        MovingAverageStrategy strategy(short_window,long_window);
        std::vector<int> out;
        for(size_t i=0;i<prices.size();++i){
            MarketData data{}; data.symbol="ASSET"; data.last_price=prices[i];
            data.timestamp=Timestamp{}+std::chrono::seconds(i+1);
            auto signals=strategy.onMarketData(data);
            out.push_back(signals.empty()?0:signals[0].side==OrderSide::BUY?1:-1);
        }
        return out;
    },py::arg("prices"),py::arg("short_window")=10,py::arg("long_window")=30);
    m.def("check_order",[](bool buy,double quantity,double price,double cash,double held,double max_weight){
        if(!std::isfinite(price) || price<=0 || !std::isfinite(quantity) || quantity<=0) return false;
        if(!std::isfinite(cash) || cash<0 || !std::isfinite(held) || held<0 || !std::isfinite(max_weight) || max_weight<=0 || max_weight>1) return false;
        Portfolio portfolio; portfolio.updateCash(cash-portfolio.getCash());
        portfolio.updatePosition("ASSET",held,price);
        RiskManager risk({max_weight,0.2,1.0,10000.,max_weight});
        risk.updateCurrentPrices({{"ASSET",price}});
        Order order("ASSET",buy?OrderSide::BUY:OrderSide::SELL,OrderType::MARKET,quantity);
        return risk.checkOrderRisk(order,portfolio);
    },py::arg("buy"),py::arg("quantity"),py::arg("price"),py::arg("cash"),py::arg("held"),py::arg("max_weight")=0.1);
}
