#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>

inline std::string now_utc()
{
    auto t = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(t);
    std::tm tm = *std::gmtime(&tt);
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S UTC");
    return ss.str();
}

inline std::string hex_pad_left(const std::string &hex, size_t width)
{
    std::string h = hex;
    if (h.rfind("0x", 0) == 0)
        h = h.substr(2);
    if (h.size() >= width)
        return h;
    return std::string(width - h.size(), '0') + h;
}

inline std::string uint64_to_hex_32(uint64_t v)
{
    std::ostringstream ss;
    ss << std::hex << v;
    return hex_pad_left(ss.str(), 64);
}

inline std::string address_to_abi(const std::string &addr)
{
    std::string a = addr;
    if (a.rfind("0x", 0) == 0)
        a = a.substr(2);
    return std::string(24, '0') + a; // 32 bytes (64 hex chars)
}