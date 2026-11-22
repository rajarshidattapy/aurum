#include "order_executor.hpp"
#include <stdexcept>
#include <cmath>
namespace trading {
// In-memory paper fills only. This class contains no broker adapter.
OrderExecutor::OrderExecutor():running_(false) {}
OrderExecutor::~OrderExecutor(){stop();}
void OrderExecutor::start(){
    std::lock_guard<std::mutex> lock(mutex_);
    if(running_) return;
    running_=true; execution_thread_=std::thread(&OrderExecutor::executionLoop,this);
}
void OrderExecutor::stop(){
    { std::lock_guard<std::mutex> lock(mutex_); running_=false;
      while(!order_queue_.empty()) {order_queue_.front()->setStatus(OrderStatus::CANCELLED);order_queue_.pop();} }
    cv_.notify_all(); if(execution_thread_.joinable()) execution_thread_.join();
}
void OrderExecutor::submitOrder(std::shared_ptr<Order> order){
    std::lock_guard<std::mutex> lock(mutex_);
    if(!running_) throw std::logic_error("Paper executor is not started");
    if(!order || !std::isfinite(order->getQuantity()) || order->getQuantity()<=0 ||
       !std::isfinite(order->getPrice()) || order->getPrice()<=0 || order->getType()!=OrderType::MARKET)
        throw std::invalid_argument("Paper fills require a positive quantity and mark price; market orders only");
    if(orders_.count(order->getOrderId())) throw std::invalid_argument("Duplicate order");
    orders_[order->getOrderId()]=order;order_queue_.push(order);cv_.notify_one();
}
void OrderExecutor::cancelOrder(const std::string& id){
    std::lock_guard<std::mutex> lock(mutex_);auto it=orders_.find(id);
    if(it==orders_.end()) throw std::out_of_range("Unknown order");
    if(it->second->getStatus()==OrderStatus::PENDING) it->second->setStatus(OrderStatus::CANCELLED);
}
OrderStatus OrderExecutor::getOrderStatus(const std::string& id){
    std::lock_guard<std::mutex> lock(mutex_);return orders_.at(id)->getStatus();
}
void OrderExecutor::executionLoop(){
    for(;;){ std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock,[this]{return !running_ || !order_queue_.empty();});
        if(!running_) return;
        auto order=order_queue_.front();order_queue_.pop();
        if(order->getStatus()==OrderStatus::PENDING) executeOrder(order);
    }
}
void OrderExecutor::executeOrder(std::shared_ptr<Order> order){
    order->setFilledQuantity(order->getQuantity());order->setStatus(OrderStatus::FILLED);
}
}
