#include "ethereum_rpc.h"
#include "curve_pool.h"
#include "order_agent.h"
#include "utils.h"
#include <iostream>
#include <thread>

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        std::cerr << "Usage: agent <rpc_url> <pool_address> <signer_post_url>
                     ";
            return 1;
    }
    std::string rpc = argv[1];
    std::string pooladdr = argv[2];
    std::string signer = argv[3];

    curl_global_init(CURL_GLOBAL_DEFAULT);
    EthereumRPC erpc(rpc);
    CurvePool pool(pooladdr, &erpc);
    OrderAgent agent(&erpc, &pool, signer);

    // place example orders
    Order o1;
    o1.owner = "0xYourAddress";
    o1.side = OrderSide::SELL;
    o1.tokenIn = "DAI";
    o1.tokenOut = "USDC";
    o1.amountIn = 1000000;
    o1.amountOutMin = 990000;
    o1.tif = TimeInForce::GTC;
    agent.place_order(o1);

    Order o2;
    o2.owner = "0xYourAddress";
    o2.side = OrderSide::SELL;
    o2.tokenIn = "DAI";
    o2.tokenOut = "USDC";
    o2.amountIn = 500000;
    o2.amountOutMin = 499000;
    o2.tif = TimeInForce::IOC;
    agent.place_order(o2);

    std::this_thread::sleep_for(std::chrono::seconds(15));
    curl_global_cleanup();
    return 0;
}