#include "packet.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

ClientPacket createPacket(int X, int Y, int V, int M, int S, double A, int P, uint16_t R) {
    ClientPacket packet{};
    
    const int scaledAcceleration = std::clamp(static_cast<int>(std::lround(A * 10.0)), -128, 127);
    int8_t A_fixed = static_cast<int8_t>(scaledAcceleration);
    
    uint8_t Y_unsigned = Y & 0x3F;
    
    packet.word1 = (Y_unsigned << 6) | (X & 0x3F);
    
    packet.word2 = ((S & 0x03) << 12) | ((M & 0x03) << 8) | (V & 0xFF);
    
    packet.word3 = ((P & 0xFF) << 8) | (A_fixed & 0xFF);
    
    packet.word4 = R;
    
    return packet;
}

void unpackPacket(const ClientPacket& packet, int& X, int& Y, int& V, int& M, int& S, double& A, int& P, uint16_t& R) {
    X = packet.word1 & 0x3F;
    uint8_t Y_unsigned = (packet.word1 >> 6) & 0x3F;
    Y = static_cast<int8_t>(Y_unsigned << 2) >> 2;
    
    V = packet.word2 & 0xFF;
    M = (packet.word2 >> 8) & 0x03;
    S = (packet.word2 >> 12) & 0x03;
    
    int8_t A_fixed = static_cast<int8_t>(packet.word3 & 0xFF);
    A = static_cast<double>(A_fixed) / 10.0;
    P = (packet.word3 >> 8) & 0xFF;
    
    R = packet.word4;
}

bool validateParameters(const ParameterLimits& limits, int X, int Y, int V, int M, int S, double A, int P) {
    return (X >= limits.X_min && X <= limits.X_max &&
            Y >= limits.Y_min && Y <= limits.Y_max &&
            V >= limits.V_min && V <= limits.V_max &&
            M >= limits.M_min && M <= limits.M_max &&
            S >= limits.S_min && S <= limits.S_max &&
            A >= limits.A_min && A <= limits.A_max &&
            P >= limits.P_min && P <= limits.P_max);
}

bool ParameterLimits::loadFromJson(const std::string& filename) {
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        json j;
        file >> j;
        
        auto limits_json = j["limits"][0];
        X_min = limits_json["X"][0];
        X_max = limits_json["X"][1];
        Y_min = limits_json["Y"][0];
        Y_max = limits_json["Y"][1];
        V_min = limits_json["V"][0];
        V_max = limits_json["V"][1];
        M_min = limits_json["M"][0];
        M_max = limits_json["M"][1];
        S_min = limits_json["S"][0];
        S_max = limits_json["S"][1];
        A_min = limits_json["A"][0];
        A_max = limits_json["A"][1];
        P_min = limits_json["P"][0];
        P_max = limits_json["P"][1];
        
        return true;
    } catch (...) {
        return false;
    }
}