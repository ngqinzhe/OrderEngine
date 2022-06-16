#ifndef orderEngine_h
#define orderEngine_h

#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <chrono>

class Order {
private:
    bool side;
    std::string orderId;
    int price;
    int quantity;
    int displayQuantity;
    int currentDisplayQuantity;
    int totalTrade;
public:
    Order(bool _side, std::string _orderId, int _quantity, int _price = 0, int _displayQuantity = 0, int _totalTrade = 0) {
        side = _side;
        orderId = _orderId;
        quantity = _quantity;
        price = _price;
        displayQuantity = _displayQuantity;
        currentDisplayQuantity = displayQuantity;
        totalTrade = _totalTrade;
    }

    // getters
    int getQuantity() const { return quantity; }
    int getDisplayQuantity() const { return displayQuantity; }
    int getCurrentDisplayQuantity() const { return currentDisplayQuantity; }
    int getPrice() const { return price; }
    bool isBuy() const { return side; }
    std::string getOrderId() const { return orderId; }
    int getTotalTraded() const { return totalTrade; }

    // setters for cancel / replace
    void setNewQuantity(int _quantity) { quantity = _quantity; }
    void setNewPrice(int _price) { price = _price; }
    void setNewOrderId(std::string _orderId) { orderId = _orderId; }

    // functionality
    void print(std::ostream& os) const;
    void reduceQuantity(int value) { quantity -= value; }
    void reduceCurrentDisplayQuantity(int value) { 
        currentDisplayQuantity -= value; 
        quantity -= value;
    }
    void traded(int value) { totalTrade += value; }
};
inline std::ostream& operator <<(std::ostream& os, Order& order) {
    order.print(os);
    return os;
}

class Orderbook {
    // type definition for the iterator for iterator mapping
    using buyOrderIter = std::multimap<int, Order, std::greater<int>>::iterator;
    using sellOrderIter = std::multimap<int, Order, std::less<int>>::iterator;

    // container for orders
    std::multimap<int, Order, std::greater<int>> buyLimit;
    std::multimap<int, Order, std::less<int>> sellLimit;

    // mapping to the orders using orderid for faster access
    std::unordered_map<std::string, buyOrderIter> buyOrderMap;
    std::unordered_map<std::string, sellOrderIter> sellOrderMap;
    
    // to log current trade for printing to stdout
    int currentTrade = 0;
public:
    void insertOrder(Order& o, bool isCRP = false, bool repeat = false);
    void matchOrder(Order& o);
    void reinsertICE(Order& o, bool isBuy);
    void removeOrder(std::string orderId, bool isBuy);
    void insertMarketOrder(Order& o);
    void cancelOrder(std::string _orderId);
    void insertIOCOrder(Order& o);
    void insertFOKOrder(Order& o);
    void cancelReplaceOrder(std::string orderId, int newQuantity, int newPrice);
    void setNewParameters(std::string orderId, int newQuantity, int newPrice, buyOrderIter buyIter, sellOrderIter sellIter);
    
    void parseInput(std::vector<std::string>& result);

    void printOrderbook(std::ostream& os);
};

inline std::ostream& operator <<(std::ostream& os, Orderbook& ob) {
    ob.printOrderbook(os);
    return os;
}

#endif