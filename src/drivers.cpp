#include "drivers.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

// =======================
// Implementação do LinuxSocketCanDriver
// =======================

LinuxSocketCanDriver::LinuxSocketCanDriver(SystemConfig cfg) 
    : config(cfg), socket_fd(-1), is_connected(false) {}

LinuxSocketCanDriver::~LinuxSocketCanDriver() {
    close_connection();
}

bool LinuxSocketCanDriver::init() {
    // A implementação é a mesma que fizemos antes, mas agora isolada aqui
    if ((socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("Socket creation failed");
        return false;
    }

    struct ifreq ifr;
    std::strcpy(ifr.ifr_name, config.interface_name.c_str());

    if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
        perror("Interface lookup failed");
        return false;
    }

    struct sockaddr_can addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return false;
    }

    apply_filters();
    is_connected = true;
    std::cout << "[DRIVER] SocketCAN iniciado na interface: " << config.interface_name << std::endl;
    return true;
}

void LinuxSocketCanDriver::apply_filters() {
    if (config.filter_ids.empty()) return;

    std::vector<struct can_filter> rfilter;
    uint32_t mask = config.use_extended_id ? CAN_EFF_MASK : CAN_SFF_MASK;

    for (uint32_t id : config.filter_ids) {
        struct can_filter f;
        f.can_id = id;
        f.can_mask = mask;
        rfilter.push_back(f);
    }
    setsockopt(socket_fd, SOL_CAN_RAW, CAN_RAW_FILTER, rfilter.data(), sizeof(struct can_filter) * rfilter.size());
}

bool LinuxSocketCanDriver::read(CanMessage& msg) {
    if (!is_connected) return false;

    struct can_frame frame;
    int nbytes = ::read(socket_fd, &frame, sizeof(struct can_frame));

    if (nbytes < 0) {
        // perror("Read error"); // Opcional: comentar para não poluir log se for timeout
        return false;
    }

    uint32_t mask = config.use_extended_id ? CAN_EFF_MASK : CAN_SFF_MASK;
    msg.id = frame.can_id & mask;
    msg.dlc = frame.can_dlc;
    std::memcpy(msg.data, frame.data, 8);

    return true;
}

void LinuxSocketCanDriver::close_connection() {
    if (socket_fd >= 0) {
        ::close(socket_fd);
        socket_fd = -1;
        is_connected = false;
    }
}

// =======================
// Implementação do MockCanDriver
// =======================

bool MockCanDriver::init() {
    std::cout << "[MOCK] Iniciado." << std::endl;
    return true;
}

bool MockCanDriver::read(CanMessage& msg) {
    sleep(1); // Simula delay
    msg.id = 0x123;
    msg.dlc = 4;
    msg.data[0] = 0xDE;
    msg.data[1] = 0xAD;
    msg.data[2] = 0xBE;
    msg.data[3] = 0xEF;
    return true;
}

void MockCanDriver::close_connection() {}

// =======================
// Factory
// =======================
std::unique_ptr<ICanDriver> createDriver(const SystemConfig& config) {
    if (config.driver_type == "socketcan") {
        return std::make_unique<LinuxSocketCanDriver>(config);
    } else {
        return std::make_unique<MockCanDriver>();
    }
}
