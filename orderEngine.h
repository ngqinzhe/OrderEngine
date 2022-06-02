#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <queue>

class IOrder {
public:
    virtual ~IOrder() {}
    // getters
    virtual int getQuantity() const = 0;
    virtual bool isBuy() const = 0;
    virtual std::string getOrderId() const = 0;
    // functionality
    virtual void reduceQuantity(int value) = 0;
};

class LimitOrder : public IOrder {
private:
    bool side;
    std::string orderId;
    int price;
    int quantity;
    int totalTrade = 0;
    int displayQuantity = 0;
public:
    explicit LimitOrder(bool _side, std::string _orderId, int _price, int _quantity, int _displayQuantity = 0) : 
        side(_side), orderId(_orderId), price(_price), quantity(_quantity), displayQuantity(_displayQuantity) {}
    // getters
    int getQuantity() const { return quantity; }
    int getDisplayQuantity() const { return displayQuantity; }
    int getPrice() const { return price; }
    bool isBuy() const { return side; }
    std::string getOrderId() const { return orderId; }
    int getTotalTrade() const { return totalTrade; }

    // setters for cancel / replace
    void setNewQuantity(int _quantity) { quantity = _quantity; }
    void setNewPrice(int _price) { price = _price; }
    void setNewOrderId(std::string _orderId) { orderId = _orderId; }

    // functionality
    void print() const;
    void reduceQuantity(int value);
    void traded(int value);

    // for ICEOrder
    void resetTraded() { totalTrade = 0; }

};

class MarketOrder : public IOrder {
    bool side;
    std::string orderId;
    int quantity;
    int totalTrade = 0;
public:
    MarketOrder(bool _side, std::string _orderId, int _quantity) :
        side(_side), orderId(_orderId), quantity(_quantity) {}
    // getters
    int getQuantity() const { return quantity; }
    bool isBuy() const { return side; }
    std::string getOrderId() const { return orderId; }
    int getTotalTraded() const { return totalTrade; }
    // functionality
    void reduceQuantity(int value);
    void traded(int value);
};

class Orderbook {
    // iterator for mapping
    typedef std::multimap<int, LimitOrder, std::greater<int>>::iterator buyOrderIter;
    typedef std::multimap<int, LimitOrder, std::less<int>>::iterator sellOrderIter;
    // container for orders
    std::multimap<int, LimitOrder, std::greater<int>> buyLimit;
    std::multimap<int, LimitOrder, std::less<int>> sellLimit;
    // mapping to the orders using orderid
    std::unordered_map<std::string, buyOrderIter> buyOrderMap;
    std::unordered_map<std::string, sellOrderIter> sellOrderMap;


public:
    void printOrderbook();
    void insertLimitOrder(LimitOrder& o, bool isCRP);
    void matchLimitOrder(LimitOrder& o);
    void removeOrder(std::string orderId, bool isBuy);
    void insertMarketOrder(MarketOrder& o);
    void cancelOrder(std::string _orderId);
    void insertIOCOrder(LimitOrder& o);
    void insertFOKOrder(LimitOrder& o);
    void cancelReplaceOrder(std::string orderId, int newQuantity, int newPrice);
    void setNewParameters(std::string orderId, int newQuantity, int newPrice, buyOrderIter buyIter, sellOrderIter sellIter);
};