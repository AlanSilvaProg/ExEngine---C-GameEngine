#include "UID.h"
#include <random>
#include <sstream>
#include <iomanip>

std::string UID::GenerateGUID() {
    // gerador global
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;

    uint64_t a = dis(gen);
    uint64_t b = dis(gen);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    // formata UUID4
    ss << std::setw(8) << (a >> 32)
       << "-"
       << std::setw(4) << ((a >> 16) & 0xFFFF)
       << "-"
       << std::setw(4) << (0x4000 | (a & 0x0FFF))         // versão 4
       << "-"
       << std::setw(4) << (0x8000 | ((b >> 48) & 0x3FFF)) // variante RFC 4122
       << "-"
       << std::setw(12) << (b & 0xFFFFFFFFFFFFULL);

    return ss.str();
};