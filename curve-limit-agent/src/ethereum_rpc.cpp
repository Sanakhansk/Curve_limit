#include "ethereum_rpc.h"
#include "utils.h"
#include <stdexcept>
#include <string>

static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t totalsz = size * nmemb;
    std::string *s = static_cast<std::string *>(userp);
    s->append(static_cast<char *>(contents), totalsz);
    return totalsz;
}

EthereumRPC::EthereumRPC(const std::string &url) : rpc_url(url)
{
    curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl init failed");
}
EthereumRPC::~EthereumRPC()
{
    if (curl)
        curl_easy_cleanup(curl);
}

json EthereumRPC::call(const std::string &method, const json &params)
{
    json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", method}, {"params", params}};
    std::string sreq = req.dump();
    std::string resp;

    struct curl_slist *hdr = nullptr;
    hdr = curl_slist_append(hdr, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, rpc_url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sreq.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdr);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    CURLcode rc = curl_easy_perform(curl);
    curl_slist_free_all(hdr);
    if (rc != CURLE_OK)
        throw std::runtime_error(std::string("curl error: ") + curl_easy_strerror(rc));
    return json::parse(resp);
}

json EthereumRPC::eth_call(const std::string &to, const std::string &data)
{
    json callobj = {{"to", to}, {"data", data}};
    return call("eth_call", json::array({callobj, "latest"}));
}
