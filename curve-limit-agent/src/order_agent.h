#pragma once
#include <cstdint>
#include <string>
#include <chrono>
#include <functional>
#include "curve_pool.h"

enum class OrderSide
{
    BUY,
    SELL
};
enum class TimeInForce
{
    GTC,
    GTT,
    IOC,
    FOK
};
enum class OrderStatus
{
    NEW,
    OPEN,
    PARTIAL,
    FILLED,
    CANCELLED,
    EXPIRED
};

struct Order
{
    uint64_t id;
    std::string owner;
    OrderSide side;
    std::string tokenIn, tokenOut;
    uint64_t amountIn;
    uint64_t amountOutMin;
    TimeInForce tif;
    std::chrono::system_clock::time_point expires_at;
    OrderStatus status;
    uint64_t filled;
};

class OrderAgent
{
public:
    OrderAgent(EthereumRPC *rpc, CurvePool *pool, const std::string &signer_post_url);
    ~OrderAgent();
    uint64_t place_order(const Order &o);
    bool cancel_order(uint64_t id);

private:
    // non-copyable
    OrderAgent(const OrderAgent &) = delete;
    OrderAgent &operator=(const OrderAgent &) = delete;
};