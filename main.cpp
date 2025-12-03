#include "uci.h"
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>

int main() {
    // Enable unbuffered output for UCI protocol compatibility
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);

    global_rng.seed(std::chrono::steady_clock::now().time_since_epoch().count());

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "uci") { handleUci(); }
        else if (command == "isready") { handleIsReady(); }
        else if (command == "ucinewgame") { handleUciNewGame(); }
        else if (command == "position") { handlePosition(iss); }
        else if (command == "go") { handleGo(iss); }
        else if (command == "quit") { break; }
    }

    return 0;
}
