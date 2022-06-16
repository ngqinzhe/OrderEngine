#include "orderEngine.h"

// printing for Individual orders
void Order::print(std::ostream& os) const { 
    if (displayQuantity) {
        os << currentDisplayQuantity << "(" << quantity << ")" << "@" << price << "#" << orderId;
    } else 
        os << quantity << "@" << price << "#" << orderId;
}

// printing for Orderbook
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
}

// insert Order into orderbook
void Orderbook::insertOrder(Order& o, bool isCRP, bool repeat) {
    if (o.isBuy()) {
        buyLimit.insert(std::pair<int, Order>(o.getPrice(), o));
        auto currIter = buyLimit.find(o.getPrice());
        while (currIter != buyLimit.end() && currIter->second.getOrderId() != o.getOrderId()) currIter++;
        buyOrderMap[o.getOrderId()] = currIter;
    }
    else {
        sellLimit.insert(std::pair<int, Order>(o.getPrice(), o));
        auto currIter = sellLimit.find(o.getPrice());
        while (currIter != sellLimit.end() && currIter->second.getOrderId() != o.getOrderId()) currIter++;
        sellOrderMap[o.getOrderId()] = currIter;
    }

    matchOrder(o);
    if (!isCRP && !repeat) {
        std::cout << currentTrade << std::endl;
        currentTrade = 0;
    }
}

// match Orders in the orderbook
void Orderbook::matchOrder(Order& o) {
    // recursively match orders

    if (sellLimit.size() == 0 || buyLimit.size() == 0
        || (buyLimit.begin()->second.getPrice() < sellLimit.begin()->second.getPrice())) {
            return;
    }
    buyOrderIter buyIter =  buyLimit.begin();
    sellOrderIter sellIter = sellLimit.begin();
    // check if either is ICE, display quantity != 0 will indicate ICE orders
    bool buyIsICE = buyIter->second.getDisplayQuantity() ? true : false;
    bool sellIsICE = sellIter->second.getDisplayQuantity() ? true : false;

    int tradeQuantity = std::min(
        buyIsICE ? 
            buyIter->second.getDisplayQuantity() : buyIter->second.getQuantity(), 
        sellIsICE ? 
            sellIter->second.getDisplayQuantity() : sellIter->second.getQuantity());
    int tradePrice = 0;
    
    // this is for recursion, in any case when an ICE order is reinserted and not at the top priority
    if (buyIter->second.getOrderId() != o.getOrderId() && sellIter->second.getOrderId() != o.getOrderId()) tradePrice = buyIter->second.getPrice();
    else tradePrice = o.isBuy() ? sellIter->second.getPrice() : buyIter->second.getPrice();

    // if it is an ICE order, we reduce both display and quantity, otherwise just reduce quantity
    buyIsICE ? buyIter->second.reduceCurrentDisplayQuantity(tradeQuantity) : buyIter->second.reduceQuantity(tradeQuantity);
    sellIsICE ? sellIter->second.reduceCurrentDisplayQuantity(tradeQuantity) : sellIter->second.reduceQuantity(tradeQuantity);

    currentTrade += tradeQuantity * tradePrice;
    
    o.traded(currentTrade);
    // For ICE orders, if display quantity = 0, delete and reinsert
    if (buyIsICE && !buyIter->second.getCurrentDisplayQuantity()) {
        reinsertICE(o, true);
        return;
    }
    else if (sellIsICE && !sellIter->second.getCurrentDisplayQuantity()) {
        reinsertICE(o, false);
        return;
    }

    // remove order from orderbook if quantity is empty
    if (buyIter->second.getQuantity() == 0) {
        removeOrder(buyIter->second.getOrderId(), true);
    }
    if (sellIter->second.getQuantity() == 0) {
        removeOrder(sellIter->second.getOrderId(), false);
    }
    
    matchOrder(o);
}

void Orderbook::reinsertICE(Order& o, bool isBuy) {
    auto it = isBuy ? buyLimit.begin() : sellLimit.begin();
    Order newICEOrder(isBuy, 
        it->second.getOrderId(),
        it->second.getQuantity(),
        it->second.getPrice(),
        it->second.getDisplayQuantity() > it->second.getQuantity() ?
            it->second.getQuantity() : it->second.getDisplayQuantity(),
        o.getTotalTraded());
    removeOrder(it->second.getOrderId(), isBuy);
    insertOrder(newICEOrder, false, true);
}

// Insert market order with no price
void Orderbook::insertMarketOrder(Order& o) {
    // Recursively match market orders if submitted order overflows
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

// Cancel the order from orderbook given the orderId
void Orderbook::cancelOrder(std::string _orderId) {
    if (sellOrderMap.find(_orderId) != sellOrderMap.end()) {
        removeOrder(_orderId, false);
    }
    else if (buyOrderMap.find(_orderId) != buyOrderMap.end()) {
        removeOrder(_orderId, true);
    }
}

// Remove order from orderbook
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

// insert IOCOrder, fill then cancel the remaining qty
void Orderbook::insertIOCOrder(Order& o) {
    std::string orderId = o.getOrderId();
    insertOrder(o, false);
    cancelOrder(orderId);
}

// insert FOKOrder, check if can be fully filled and execute, else cancel
void Orderbook::insertFOKOrder(Order& o) {
    int fokOrderQuantity = o.getQuantity();
    int comparableQuantity = 0;
    // check if order can be fufilled
    auto startIter = o.isBuy() ? sellLimit.begin() : buyLimit.begin();
    auto endIter = o.isBuy() ? sellLimit.end() : buyLimit.end();

    for (; startIter != endIter; startIter++) {
        if (o.isBuy() && o.getPrice() >= startIter->second.getPrice()) comparableQuantity += startIter->second.getQuantity();
        else if (startIter->second.getPrice() >= o.getPrice()) comparableQuantity += startIter->second.getQuantity();
    }

    if (comparableQuantity < fokOrderQuantity) {
        std::cout << "0" << std::endl;
        return;
    }
    else insertOrder(o, false);
}

// cancel and replace order, if price is the same and quantity decrease, else remove and insert
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
            i += 3;
        }
        else if (result[i] == "SUB") {
            std::string orderType = result[i + 1];
            bool s = result[i+2] == "B";
            std::string n = result[i + 3];
            int q = std::stoi(result[i + 4]);
            if (orderType == "MO") {
                Order order(s, n, q);
                this->insertMarketOrder(order);
                std::cout << order.getTotalTraded() << std::endl;
                i += 4;
            } 
            else if (orderType == "ICE") {
                int p = std::stoi(result[i + 5]);
                int d = std::stoi(result[i + 6]);
                Order order(s, n, q, p, d);
                this->insertOrder(order, false);
                i += 6;
            }
            else {
                int p = std::stoi(result[i + 5]);
                Order order(s, n, q, p);
                if (orderType == "LO") this->insertOrder(order);
                if (orderType == "IOC") this->insertIOCOrder(order);
                if (orderType == "FOK") this->insertFOKOrder(order);
                i += 5;
            }
        }
    }
}