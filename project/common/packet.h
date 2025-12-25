#ifndef PACKET_H
#define PACKET_H

#include <cstdint>
#include <vector>
#include <string>
#include <cmath>

#pragma pack(push, 1)
struct ClientPacket {
    uint16_t word1;
    uint16_t word2;
    uint16_t word3;
    uint16_t word4;
};

struct ServerResponse {
    uint8_t messageId;
    uint8_t isValid;
};
#pragma pack(pop)

struct ParameterLimits {
    int X_min = 0, X_max = 63;
    int Y_min = -32, Y_max = 31;
    int V_min = 0, V_max = 255;
    int M_min = 0, M_max = 3;
    int S_min = 0, S_max = 3;
    double A_min = -12.7, A_max = 12.8;
    int P_min = 0, P_max = 130;

    ParameterLimits() = default;
    
    bool loadFromJson(const std::string& filename);
};

ClientPacket createPacket(int X, int Y, int V, int M, int S, double A, int P, uint16_t R = 0);
void unpackPacket(const ClientPacket& packet, int& X, int& Y, int& V, int& M, int& S, double& A, int& P, uint16_t& R);
bool validateParameters(const ParameterLimits& limits, int X, int Y, int V, int M, int S, double A, int P);

#endif