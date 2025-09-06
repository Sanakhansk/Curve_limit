#pragma once
#include <string>
#include <cstdint>
#include "ethereum_rpc.h"

class CurvePool
{
public:
    CurvePool(const std::string &addr, EthereumRPC *rpc);
    // simple get_dy wrapper for pool get_dy(i,j,dx)
    uint64_t get_dy(int64_t i, int64_t j, uint64_t dx);
    std::string address() const;

private:
    std::string pool_addr;
    EthereumRPC *rpc;
};