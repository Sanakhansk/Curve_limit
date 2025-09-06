#include "curve_pool.h"
#include "utils.h"
#include <stdexcept>

CurvePool::CurvePool(const std::string &addr, EthereumRPC *r) : pool_addr(addr), rpc(r) {}
std::string CurvePool::address() const { return pool_addr; }

uint64_t CurvePool::get_dy(int64_t i, int64_t j, uint64_t dx)
{
    // selector for get_dy(int128,int128,uint256) => 0x5e0d443f
    std::string sel = "0x5e0d443f";
    std::string data = sel + uint64_to_hex_32(static_cast<uint64_t>(i)) + uint64_to_hex_32(static_cast<uint64_t>(j)) + uint64_to_hex_32(dx);
    auto res = rpc->eth_call(pool_addr, data);
    if (res.contains("error"))
        throw std::runtime_error("eth_call error");
    std::string r = res["result"].get<std::string>();
    if (r.rfind("0x", 0) == 0)
        r = r.substr(2);
    if (r.size() < 64)
        return 0;
    std::string last = r.substr(r.size() - 64);
    try
    {
        return std::stoull(last, nullptr, 16);
    }
    catch (...)
    {
        return 0;
    }
}