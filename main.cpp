#include "orderEngine.h"
#include <iomanip>
#include <chrono>

int main() {
    Orderbook ob;

    std::vector<std::string> result;
    std::string input;
    while (std::cin >> input) {
        if (input == "END") break;
        result.push_back(input);
    }
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < result.size(); i++) {
        if (result[i] == "CXL") {
            std::string orderId = result[i + 1];
            ob.cancelOrder(orderId);
            i += 1;
            continue;
        }
        else if (result[i] == "CRP") {
            std::string orderId = result[i + 1];
            int newQuantity = std::stoi(result[i + 2]);
            int newPrice = std::stoi(result[i + 3]);
            ob.cancelReplaceOrder(orderId, newQuantity, newPrice);
        }
        else if (result[i] == "SUB") {
            std::string orderType = result[i + 1];
            bool s = result[i+2] == "B";
            std::string n = result[i + 3];
            int q = std::stoi(result[i + 4]);
            if (orderType == "LO") {
                int p = std::stoi(result[i + 5]);
                LimitOrder order(s, n, p, q);
                ob.insertLimitOrder(order, false);
                i += 5;
            }
            else if (orderType == "MO") {
                MarketOrder order(s, n, q);
                ob.insertMarketOrder(order);
                i += 4;
            }
            else if (orderType == "IOC") {
                int p = std::stoi(result[i + 5]);
                LimitOrder order(s, n, p, q);
                ob.insertIOCOrder(order);
                i += 5;
            }
            else if (orderType == "FOK") {
                int p = std::stoi(result[i + 5]);
                LimitOrder order(s, n, p, q);
                ob.insertFOKOrder(order);
                i += 5;
            }
            else if (orderType == "ICE") {
                int p = std::stoi(result[i + 5]);
                int d = std::stoi(result[i + 6]);
                LimitOrder order(s, n, p, q, d);
                ob.insertLimitOrder(order, false);
                i += 6;
            }
        }
    }

    std::cout << ob << std::endl;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Execution Time: " << std::chrono::duration_cast<std::chrono::microseconds> (end - begin).count() << "[ms]" << std::endl;
}