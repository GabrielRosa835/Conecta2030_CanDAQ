#pragma once
#include <string>
#include <vector>
#include <cstdint>

struct SystemConfig {
    std::string interface_name;
    std::string driver_type;
    std::vector<uint32_t> filter_ids;
    bool use_extended_id;
    bool human_readable; // NOVO CAMPO
};

SystemConfig loadConfig(const std::string& filename);
