#include "core/Game.h"
#include <iostream>

int main() {
    std::cout << "[main] Starting..." << std::endl;
    try {
        Game::get().init();
        std::cout << "[main] Init done, running..." << std::endl;
        Game::get().run();
    } catch (const std::exception& e) {
        std::cerr << "[main] Exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[main] Unknown exception!" << std::endl;
    }
    std::cout << "[main] Exiting." << std::endl;
    return 0;
}
