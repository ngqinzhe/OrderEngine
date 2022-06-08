#ifndef orderEngine_h
#define orderEngine_h

#pragma once
#include <iostream>
#include <vector>
#include <map>
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
    int totalTrade = 0;
    int displayQuantity;
public:
    Order(bool _side, std::string _orderId, int _quantity, int _price = 0, int _displayQuantity = 0) {
        side = _side;
        orderId = _orderId;
        quantity = _quantity;
        price = _price;
        displayQuantity = _displayQuantity;
    }
    // getters
    int getQuantity() const { return quantity; }
    int getDisplayQuantity() const { return displayQuantity; }
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
    void traded(int value) { totalTrade += value; }

    // for ICEOrder
    void resetTraded() { totalTrade = 0; }
};
inline std::ostream& operator <<(std::ostream& os, Order& order) {
    order.print(os);
    return os;
}

class Orderbook {
    // iterator for mapping
    typedef std::multimap<int, Order, std::greater<int>>::iterator buyOrderIter;
    typedef std::multimap<int, Order, std::less<int>>::iterator sellOrderIter;
    // container for orders
    std::multimap<int, Order, std::greater<int>> buyLimit;
    std::multimap<int, Order, std::less<int>> sellLimit;
    // mapping to the orders using orderid
    std::unordered_map<std::string, buyOrderIter> buyOrderMap;
    std::unordered_map<std::string, sellOrderIter> sellOrderMap;


public:
    void printOrderbook(std::ostream& os);
    void insertOrder(Order& o, bool isCRP);
    void matchOrder(Order& o);
    void removeOrder(std::string orderId, bool isBuy);
    void insertMarketOrder(Order& o);
    void cancelOrder(std::string _orderId);
    void insertIOCOrder(Order& o);
    void insertFOKOrder(Order& o);
    void cancelReplaceOrder(std::string orderId, int newQuantity, int newPrice);
    void setNewParameters(std::string orderId, int newQuantity, int newPrice, buyOrderIter buyIter, sellOrderIter sellIter);
    void parseInput(std::vector<std::string>& result);
};

inline std::ostream& operator <<(std::ostream& os, Orderbook& ob) {
    ob.printOrderbook(os);
    return os;
}


#endif