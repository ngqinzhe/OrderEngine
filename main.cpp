#include "orderEngine.h"

int main() {
    Orderbook ob;

    std::vector<std::string> result;
    std::string input;
    while (std::cin >> input) {
        if (input == "END") break;
        result.push_back(input);
    }
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    ob.parseInput(result);

    std::cout << ob << std::endl;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Execution Time: " << std::chrono::duration_cast<std::chrono::microseconds> (end - begin).count() << "[ms]" << std::endl;
}