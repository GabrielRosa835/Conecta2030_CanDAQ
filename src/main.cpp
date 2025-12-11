#include <iostream>
#include "config.hpp"
#include "drivers.hpp"
#include "can_defs.hpp" // Para usar CanProtocol

int main() {
    try {
        SystemConfig config = loadConfig("config.json");
        auto driver = createDriver(config);

        if (!driver->init()) return 1;

        std::cout << "DAQ Iniciado. Modo Legivel: " << (config.human_readable ? "SIM" : "NAO") << std::endl;
        
        CanMessage frame;
        while (true) {
            if (driver->read(frame)) {
                if (config.human_readable) {
                    std::cout << "[RX] " << CanProtocol::decode(frame) << std::endl;
                } else {
                    std::cout << "[RX] " << CanProtocol::formatRaw(frame) << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "ERRO FATAL: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
