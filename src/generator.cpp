#include <iostream>
#include <vector>
#include <cstring>
#include <random>
#include <thread>
#include <chrono>

// Includes do Sistema
#include "config.hpp"     // Lê o config.json
#include "can_defs.hpp"   // Usa CanMessage e IDs
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

class CanSender {
private:
    int socket_fd;
    SystemConfig config; // Guarda a config carregada
    std::mt19937 rng;

public:
    CanSender(SystemConfig cfg) : config(cfg) {
        std::random_device rd;
        rng.seed(rd());
    }

    ~CanSender() {
        if (socket_fd >= 0) close(socket_fd);
    }

    bool init() {
        if ((socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
            perror("Erro ao criar socket");
            return false;
        }

        struct ifreq ifr;
        // Usa o nome da interface vindo do JSON
        std::strcpy(ifr.ifr_name, config.interface_name.c_str());
        
        if (ioctl(socket_fd, SIOCGIFINDEX, &ifr) < 0) {
            perror("Interface invalida (verifique config.json)");
            return false;
        }

        struct sockaddr_can addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            perror("Erro no bind");
            return false;
        }
        return true;
    }

    void send(const CanMessage& msg) {
        struct can_frame frame;
        frame.can_id = msg.id;
        frame.can_dlc = msg.dlc;
        std::memcpy(frame.data, msg.data, msg.dlc);

        if (write(socket_fd, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
            perror("Erro envio");
        } else {
            // Usa a flag do JSON para decidir como printar
            if (config.human_readable) {
                 std::cout << "[TX] " << CanProtocol::decode(msg) << std::endl;
            } else {
                 std::cout << "[TX] " << CanProtocol::formatRaw(msg) << std::endl;
            }
        }
    }

    int random_int(int min, int max) {
        std::uniform_int_distribution<int> uni(min, max);
        return uni(rng);
    }
};

int main() {
    try {
        // Carrega a configuração do JSON
        SystemConfig config = loadConfig("config.json");
        
        CanSender sender(config);
        if (!sender.init()) return 1;

        std::cout << "Gerador Iniciado na interface: " << config.interface_name << std::endl;

        static int counter = 0; // Contador de ciclos

        while (true) {
            counter++; // Incrementa o ciclo global

            // ---------------------------------------------------------
            // 1. Motor (Enviado a cada ciclo - 5Hz)
            // ---------------------------------------------------------
            CanMessage motorMsg;
            motorMsg.id = ID_MOTOR;
            motorMsg.dlc = 8; // Define explicitamente que estamos usando os primeiros bytes
            
            int rpm = sender.random_int(800, 4500);
            int vel = sender.random_int(0, 140);
            int temp = sender.random_int(80, 105);

            if (counter % 2 == 0) {
            
            motorMsg.data[0] = (rpm >> 8) & 0xFF;
            motorMsg.data[1] = rpm & 0xFF;
            motorMsg.data[2] = (uint8_t)vel;
            motorMsg.data[3] = (uint8_t)temp;
            
            sender.send(motorMsg);
            }

            // ---------------------------------------------------------
            // 2. Info da Placa (Enviado a cada 10 ciclos)
            // ---------------------------------------------------------
            if (counter % 3 == 0) {
                CanMessage infoMsg;
                infoMsg.id = ID_IDENTIFICACAO;
                std::string placa = "ABC1234";
                infoMsg.dlc = placa.size();
                std::memcpy(infoMsg.data, placa.c_str(), infoMsg.dlc);
                sender.send(infoMsg);
            }

            // ---------------------------------------------------------
            // 3. Sistema Elétrico/Fabricação (Enviado a cada 20 ciclos)
            // ---------------------------------------------------------
            if (counter % 5 == 0) {
                CanMessage elecMsg;
                elecMsg.id = ID_ELETRICA;
                elecMsg.dlc = 4; // Ano(2) + Mes(1) + Dia(1)

                uint16_t ano = 2024;
                uint8_t mes = 12;
                uint8_t dia = 10;

                // Big Endian para o Ano
                elecMsg.data[0] = (ano >> 8) & 0xFF;
                elecMsg.data[1] = ano & 0xFF;
                elecMsg.data[2] = mes;
                elecMsg.data[3] = dia;

                sender.send(elecMsg);
            }

            // Espera 200ms
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    } catch (const std::exception& e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }
}
