#include "config.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

// Biblioteca JSON (Assumindo que json.hpp está disponível no sistema ou na pasta include)
#include <nlohmann/json.hpp> 
using json = nlohmann::json;

SystemConfig loadConfig(const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open()) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo: " + filename);
    }
    json j;
    f >> j;

    SystemConfig cfg;
    cfg.interface_name = j.value("interface", "vcan0");
    cfg.driver_type = j.value("driver", "socketcan");
    cfg.use_extended_id = j.value("extended", false);
    cfg.human_readable = j.value("human_readable", false); // Default false
    
    if (j.contains("filters")) {
        for (auto& element : j["filters"]) {
            cfg.filter_ids.push_back(element);
        }
    }
    return cfg;
}
