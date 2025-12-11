#pragma once

#include "config.hpp"
#include <cstdint>
#include <memory> // Para std::unique_ptr
#include "can_defs.hpp"

// Headers do Linux necessários para a definição da classe
// O #ifdef __linux__ evita erros de syntax highlight se você abrir isso no Windows
#ifdef __linux__
    #include <net/if.h>
    #include <linux/can.h>
    #include <linux/can/raw.h>
#else
    // Definições dummy apenas para não quebrar intellisense em outros OS
    typedef int canid_t;
#endif

// Interface Abstrata
class ICanDriver {
public:
    virtual ~ICanDriver() = default;
    virtual bool init() = 0;
    virtual bool read(CanMessage& msg) = 0;
    virtual void close_connection() = 0;
};

// Driver Real (SocketCAN)
class LinuxSocketCanDriver : public ICanDriver {
private:
    int socket_fd;
    SystemConfig config;
    bool is_connected;

    void apply_filters();

public:
    LinuxSocketCanDriver(SystemConfig cfg);
    ~LinuxSocketCanDriver();

    bool init() override;
    bool read(CanMessage& msg) override;
    void close_connection() override;
};

// Driver Mock
class MockCanDriver : public ICanDriver {
public:
    bool init() override;
    bool read(CanMessage& msg) override;
    void close_connection() override;
};

// Factory Function para criar o driver correto
std::unique_ptr<ICanDriver> createDriver(const SystemConfig& config);
