cmake_minimum_required(VERSION 3.10)
project(CanDaqSystem)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include_directories(include)
find_package(nlohmann_json QUIET)

# DAQ: Usa main, config e drivers
add_executable(can_daq 
    src/main.cpp 
    src/config.cpp 
    src/drivers.cpp
)

# Generator: Agora usa generator E config.cpp
add_executable(can_generator 
    src/generator.cpp
    src/config.cpp   # <--- ADICIONADO AQUI
)

if(nlohmann_json_FOUND)
    target_link_libraries(can_daq PRIVATE nlohmann_json::nlohmann_json)
    target_link_libraries(can_generator PRIVATE nlohmann_json::nlohmann_json)
endif()
