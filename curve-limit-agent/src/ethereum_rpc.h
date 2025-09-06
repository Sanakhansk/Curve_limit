#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
using json = nlohmann::json;

class EthereumRPC
{
public:
    EthereumRPC(const std::string &url);
    ~EthereumRPC();
    json call(const std::string &method, const json &params);
    json eth_call(const std::string &to, const std::string &data);

private:
    std::string rpc_url;
    CURL *curl;
};