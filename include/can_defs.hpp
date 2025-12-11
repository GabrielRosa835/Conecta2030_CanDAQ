#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

// IDs Definidos no Sistema
constexpr uint32_t ID_MOTOR = 0x123;
constexpr uint32_t ID_ELETRICA = 0x456;
constexpr uint32_t ID_IDENTIFICACAO = 0x700;

// O DTO (Data Transfer Object) agora mora aqui
struct CanMessage {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
};

// Classe utilitária estática para formatação e decodificação
class CanProtocol {
public:
    static std::string decode(const CanMessage& msg) {
        std::stringstream ss;
        
        switch (msg.id) {
            case ID_MOTOR: {
                // Byte 0 e 1: RPM (Big Endian) -> (B0 << 8) | B1
                // Byte 2: Velocidade
                // Byte 3: Temperatura
                uint16_t rpm = (msg.data[0] << 8) | msg.data[1];
                uint8_t velocidade = msg.data[2];
                uint8_t temperatura = msg.data[3];
                
                ss << "[MOTOR] RPM: " << rpm 
                   << " | Vel: " << (int)velocidade << " km/h"
                   << " | Temp: " << (int)temperatura << " C";
                break;
            }
            case ID_ELETRICA: {
                // Byte 0 e 1: Ano, Byte 2: Mes, Byte 3: Dia
                uint16_t ano = (msg.data[0] << 8) | msg.data[1];
                uint8_t mes = msg.data[2];
                uint8_t dia = msg.data[3];
                ss << "[ELETRICA] Fabricacao: " << (int)dia << "/" << (int)mes << "/" << ano;
                break;
            }
            case ID_IDENTIFICACAO: {
                // ASCII String
                ss << "[INFO] Placa: ";
                for(int i=0; i < msg.dlc; i++) {
                    ss << (char)msg.data[i];
                }
                break;
            }
            default:
                ss << "[DESCONHECIDO] ID: 0x" << std::hex << msg.id;
                break;
        }
        return ss.str();
    }

    static std::string formatRaw(const CanMessage& msg) {
        std::stringstream ss;
        ss << "ID: 0x" << std::hex << std::uppercase << msg.id 
           << " | DLC: " << std::dec << (int)msg.dlc << " | Data: ";
        for(int i=0; i < msg.dlc; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)msg.data[i] << " ";
        }
        return ss.str();
    }
};
