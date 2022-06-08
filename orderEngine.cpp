#include "orderEngine.h"

/**
 * @brief Implementation for Order class
 */
void Order::print(std::ostream& os) const { 
    if (displayQuantity) {
        os << displayQuantity << "(" << quantity << ")" << "@" << price << "#" << orderId;
    } else 
        os << quantity << "@" << price << "#" << orderId;
}

/**
 * @brief Implementation for Orderbook class
 * 
 */
void Orderbook::printOrderbook(std::ostream& os) {
    std::cout << "B: ";
    for (auto it = buyLimit.begin(); it != buyLimit.end(); it++) {
        std::cout << it->second << " ";
    }
    std::cout << std::endl;
    std::cout << "S: ";
    for (auto it = sellLimit.begin(); it != sellLimit.end(); it++) {
        std::cout << it->second << " ";
    }
    std::cout << std::endl;
}

void Orderbook::insertOrder(Order& o, bool isCRP) {
    if (o.isBuy()) {
        buyLimit.insert(std::pair<int, Order>(o.getPrice(), o));
        auto currIter = buyLimit.find(o.getPrice());
        while (currIter->second.getOrderId() != o.getOrderId()) currIter++;
        buyOrderMap[o.getOrderId()] = currIter;
    }
    else {
        sellLimit.insert(std::pair<int, Order>(o.getPrice(), o));
        auto currIter = sellLimit.find(o.getPrice());
        while (currIter->second.getOrderId() != o.getOrderId()) currIter--;
        sellOrderMap[o.getOrderId()] = currIter;
    }
    bool isICE = false;
    matchOrder(o, isICE);
    if (!isCRP && !isICE) std::cout << o.getTotalTraded() << std::endl;
}

void Orderbook::matchOrder(Order& o, bool& isICE) {
    if (sellLimit.size() == 0 || buyLimit.size() == 0
        || !(buyLimit.begin()->second.getPrice() >= sellLimit.begin()->second.getPrice())) {
            return;
    }

    buyOrderIter buyIter =  buyLimit.begin();
    sellOrderIter sellIter = sellLimit.begin();
    // check if either is ICE
    isICE = buyIter->second.getDisplayQuantity() || sellIter->second.getDisplayQuantity() ? true : false;
    int tradeQuantity = std::min(
        buyIter->second.getDisplayQuantity() ? buyIter->second.getDisplayQuantity() : buyIter->second.getQuantity(), 
        sellIter->second.getDisplayQuantity() ? sellIter->second.getDisplayQuantity() : sellIter->second.getQuantity());
    int tradePrice = o.isBuy() ? sellIter->second.getPrice() : buyIter->second.getPrice();
    buyIter->second.reduceQuantity(tradeQuantity);
    sellIter->second.reduceQuantity(tradeQuantity);
    if (buyIter->second.getQuantity() == 0) {
        removeOrder(buyIter->second.getOrderId(), true);
    }
    if (sellIter->second.getQuantity() == 0) {
        removeOrder(sellIter->second.getOrderId(), false);
    }
    o.traded(tradeQuantity * tradePrice);
    if (isICE) std::cout << tradeQuantity * tradePrice << std::endl;
    matchOrder(o, isICE);
}

void Orderbook::insertMarketOrder(Order& o) {
    // BASE CASE 
    if ((o.getQuantity() == 0) || (o.isBuy() && sellLimit.size() == 0) || (!o.isBuy() && buyLimit.size() == 0)) 
        return;

    if (o.isBuy() && sellLimit.size() != 0) {
        sellOrderIter it = sellLimit.begin();
        int tradeQuantity = std::min(o.getQuantity(), it->second.getQuantity());
        int tradePrice = it->second.getPrice();
        it->second.reduceQuantity(tradeQuantity);
        o.reduceQuantity(tradeQuantity);
        o.traded(tradeQuantity * tradePrice);
        if (it->second.getQuantity() == 0) sellLimit.erase(it);      
    }   
    else if (buyLimit.size() != 0) {
        buyOrderIter it = buyLimit.begin();
        int tradeQuantity = std::min(o.getQuantity(), it->second.getQuantity());
        int tradePrice = it->second.getPrice();
        it->second.reduceQuantity(tradeQuantity);
        o.reduceQuantity(tradeQuantity);
        o.traded(tradeQuantity * tradePrice);
        if (it->second.getQuantity() == 0) buyLimit.erase(it);
    }

    insertMarketOrder(o);
}

void Orderbook::cancelOrder(std::string _orderId) {
    if (sellOrderMap.find(_orderId) != sellOrderMap.end()) {
        removeOrder(_orderId, false);
    }
    else if (buyOrderMap.find(_orderId) != buyOrderMap.end()) {
        removeOrder(_orderId, true);
    }
}

void Orderbook::removeOrder(std::string orderId, bool isBuy) {
    if (isBuy) {
        buyLimit.erase(buyOrderMap[orderId]);
        buyOrderMap.erase(orderId);
    }
    else {
        sellLimit.erase(sellOrderMap[orderId]);
        sellOrderMap.erase(orderId);
    }
}

void Orderbook::insertIOCOrder(Order& o) {
    std::string orderId = o.getOrderId();
    insertOrder(o, false);
    cancelOrder(orderId);
}

void Orderbook::insertFOKOrder(Order& o) {
    int fokOrderQuantity = o.getQuantity();
    int comparableQuantity = 0;
    // check if order can be fufilled
    if (o.isBuy()) {
        for (auto it = sellLimit.begin(); it != sellLimit.end(); it++) {
            if (o.getPrice() >= it->second.getPrice()) comparableQuantity += it->second.getQuantity();
        }
    }
    else {
        for (auto it = buyLimit.begin(); it != buyLimit.end(); it++) {
            if (it->second.getPrice() >= o.getPrice()) comparableQuantity += it->second.getQuantity();
        }
    }

    if (comparableQuantity < fokOrderQuantity) {
        std::cout << "0" << std::endl;
        return;
    }
    else insertOrder(o, false);
}

void Orderbook::cancelReplaceOrder(std::string orderId, int newQuantity, int newPrice) {
    if (buyOrderMap.find(orderId) != buyOrderMap.end()) {
        buyOrderIter it = buyOrderMap[orderId];
        if (it->second.getPrice() == newPrice && it->second.getQuantity() >= newQuantity) {
            setNewParameters(orderId, newQuantity, newPrice, it, sellLimit.end());
        } else {
            Order newOrder(true, orderId, newQuantity, newPrice);
            removeOrder(orderId, true);
            insertOrder(newOrder, true);
        }
    }
    else if (sellOrderMap.find(orderId) != sellOrderMap.end()) {
        sellOrderIter it = sellOrderMap[orderId];
        if (it->second.getPrice() == newPrice && it->second.getQuantity() >= newQuantity) {
            setNewParameters(orderId, newQuantity, newPrice, buyLimit.end(), it);
        } else {
            Order newOrder(false, orderId, newQuantity, newPrice);
            removeOrder(orderId, false);
            insertOrder(newOrder, true);
        }
    }
}

void Orderbook::setNewParameters(std::string orderId, int newQuantity, int newPrice, buyOrderIter buyIter, sellOrderIter sellIter) {
    auto it = buyIter == buyLimit.end() ? sellIter : buyIter;
    it->second.setNewOrderId(orderId);
    it->second.setNewPrice(newPrice);
    it->second.setNewQuantity(newQuantity);
}

void Orderbook::parseInput(std::vector<std::string>& result) {
    for (int i = 0; i < result.size(); i++) {
        if (result[i] == "CXL") {
            std::string orderId = result[i + 1];
            this->cancelOrder(orderId);
            i += 1;
            continue;
        }
        else if (result[i] == "CRP") {
            std::string orderId = result[i + 1];
            int newQuantity = std::stoi(result[i + 2]);
            int newPrice = std::stoi(result[i + 3]);
            this->cancelReplaceOrder(orderId, newQuantity, newPrice);
        }
        else if (result[i] == "SUB") {
            std::string orderType = result[i + 1];
            bool s = result[i+2] == "B";
            std::string n = result[i + 3];
            int q = std::stoi(result[i + 4]);
            if (orderType == "LO") {
                int p = std::stoi(result[i + 5]);
                Order order(s, n, q, p);
                this->insertOrder(order, false);
                i += 5;
            }
            else if (orderType == "MO") {
                Order order(s, n, q);
                this->insertMarketOrder(order);
                std::cout << order.getTotalTraded() << std::endl;
                i += 4;
            }
            else if (orderType == "IOC") {
                int p = std::stoi(result[i + 5]);
                Order order(s, n, q, p);
                this->insertIOCOrder(order);
                i += 5;
            }
            else if (orderType == "FOK") {
                int p = std::stoi(result[i + 5]);
                Order order(s, n, q, p);
                this->insertFOKOrder(order);
                i += 5;
            }
            else if (orderType == "ICE") {
                int p = std::stoi(result[i + 5]);
                int d = std::stoi(result[i + 6]);
                Order order(s, n, q, p, d);
                this->insertOrder(order, false);
                i += 6;
            }
        }
    }
}