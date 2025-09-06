#include "order_agent.h"
#include <map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <fstream>
#include "utils.h"

using json = nlohmann::json;

struct OrderAgentImpl
{
    std::map<uint64_t, Order> orders;
    CurvePool *pool;
    EthereumRPC *rpc;
    std::mutex mu;
    std::condition_variable cv;
    uint64_t next_id = 1;
    bool running = true;
    std::thread worker;
    std::string signer_url;
};

static size_t write_cb_local(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalsz = size * nmemb;
    std::string *s = static_cast<std::string *>(userp);
    s->append(static_cast<char *>(contents), totalsz);
    return totalsz;
}

OrderAgent::OrderAgent(EthereumRPC *rpc, CurvePool *pool, const std::string &signer_post_url)
{
    auto impl = new OrderAgentImpl();
    impl->rpc = rpc;
    impl->pool = pool;
    impl->signer_url = signer_post_url;
    impl->worker = std::thread([impl]()
                               {
        while(impl->running) {
            std::vector<uint64_t> ids;
            {
                std::unique_lock<std::mutex> lk(impl->mu);
                impl->cv.wait_for(lk, std::chrono::milliseconds(500));
                if(!impl->running) break;
                auto now = std::chrono::system_clock::now();
                for(auto &p: impl->orders) {
                    auto &o = p.second;
                    if((o.status==OrderStatus::OPEN || o.status==OrderStatus::PARTIAL)) {
                        if(o.tif==TimeInForce::GTT && now>=o.expires_at) { o.status = OrderStatus::EXPIRED; }
                        else ids.push_back(p.first);
                    }
                }
            }
            for(auto id: ids) {
                // process each order
                Order snapshot;
                {
                    std::unique_lock<std::mutex> lk(impl->mu);
                    snapshot = impl->orders[id];
                }
                uint64_t remaining = snapshot.amountIn - snapshot.filled;
                if(remaining==0) { std::unique_lock<std::mutex> lk(impl->mu); impl->orders[id].status = OrderStatus::FILLED; continue; }
                uint64_t dy = impl->pool->get_dy(0,1,remaining);
                if(dy < snapshot.amountOutMin) {
                    if(snapshot.tif==TimeInForce::IOC) {
                        if(snapshot.filled==0) { std::unique_lock<std::mutex> lk(impl->mu); impl->orders[id].status = OrderStatus::CANCELLED; }
                        continue;
                    } else if(snapshot.tif==TimeInForce::FOK) {
                        std::unique_lock<std::mutex> lk(impl->mu); impl->orders[id].status = OrderStatus::CANCELLED; continue;
                    } else continue; // GTC/GTT wait
                }
                // prepare data - placeholder selector and encoding (user must replace with pool's swap selector)
                std::string sel = "0x2e95b6c8"; // replace for real pool
                std::string data = sel + uint64_to_hex_32(0) + uint64_to_hex_32(1) + uint64_to_hex_32(remaining);

                // send to signer
                json payload = { {"to", impl->pool->address()}, {"data", data}, {"value", "0x0"} };
                CURL *c = curl_easy_init(); std::string resp; struct curl_slist *h=nullptr;
                if(!c) continue;
                h = curl_slist_append(h, "Content-Type: application/json");
                std::string p = payload.dump();
                curl_easy_setopt(c, CURLOPT_URL, impl->signer_url.c_str());
                curl_easy_setopt(c, CURLOPT_POSTFIELDS, p.c_str());
                curl_easy_setopt(c, CURLOPT_HTTPHEADER, h);
                curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_cb_local);
                curl_easy_setopt(c, CURLOPT_WRITEDATA, &resp);
                CURLcode rc = curl_easy_perform(c);
                curl_slist_free_all(h); curl_easy_cleanup(c);
                if(rc != CURLE_OK) continue;
                try {
                    json r = json::parse(resp);
                    if(r.contains("txHash")) {
                        std::unique_lock<std::mutex> lk(impl->mu);
                        impl->orders[id].filled += remaining;
                        if(impl->orders[id].filled >= impl->orders[id].amountIn) impl->orders[id].status = OrderStatus::FILLED;
                        else impl->orders[id].status = OrderStatus::PARTIAL;
                    }
                } catch(...){}
            }
        } });
    // attach impl pointer to `this` by placement in the object (cheat: store as void*)
    *reinterpret_cast<OrderAgentImpl **>(this) = impl;
}

OrderAgent::~OrderAgent()
{
    OrderAgentImpl *impl = *reinterpret_cast<OrderAgentImpl **>(this);
    impl->running = false;
    impl->cv.notify_all();
    if (impl->worker.joinable())
        impl->worker.join();
    delete impl;
}

uint64_t OrderAgent::place_order(const Order &o)
{
    OrderAgentImpl *impl = *reinterpret_cast<OrderAgentImpl **>(this);
    std::unique_lock<std::mutex> lk(impl->mu);
    Order copy = o;
    copy.id = impl->next_id++;
    copy.status = OrderStatus::OPEN;
    copy.filled = 0;
    impl->orders[copy.id] = copy;
    impl->cv.notify_all();
    // simple persistence
    json j;
    for (auto &p : impl->orders)
    {
        j[std::to_string(p.first)] = {{"id", p.second.id}, {"status", (int)p.second.status}, {"filled", p.second.filled}};
    }
    std::ofstream f("orders_snapshot.json");
    f << j.dump(2);
    f.close();
    return copy.id;
}

bool OrderAgent::cancel_order(uint64_t id)
{
    OrderAgentImpl *impl = *reinterpret_cast<OrderAgentImpl **>(this);
    std::unique_lock<std::mutex> lk(impl->mu);
    auto it = impl->orders.find(id);
    if (it == impl->orders.end())
        return false;
    if (it->second.status == OrderStatus::FILLED)
        return false;
    it->second.status = OrderStatus::CANCELLED;
    return true;
}