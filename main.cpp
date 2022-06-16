#include "orderEngine.h"

int main() {
    Orderbook ob;

    std::vector<std::string> result;
    std::string input;
    while (std::cin >> input) {
        if (input == "END") break;
        result.push_back(input);
    }
    ob.parseInput(result);

    std::cout << ob << std::endl;
}