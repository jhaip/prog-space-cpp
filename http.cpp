#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include "httplib.h"
#include <nlohmann/json.hpp>
using nlohmann::json;

std::optional<json> do_http_request()
{
    try
    {
        // HTTPS
        httplib::Client cli("https://api.darksky.net");
        cli.enable_server_certificate_verification(false);

        auto res = cli.Get("/forecast/ff4210a6ee0e933946c817939138eb1f/42.3601,-71.0589?exclude=minutely,hourly,alerts,flags");
        if (res) {
            std::cout << res->status << std::endl;
            std::cout << res->body << std::endl;
            auto j3 = json::parse(res->body);
            return j3;
        } else {
            std::cout << "bad response" << res.error() << std::endl;
            return {};
        }
    }
    catch (const std::exception &e) // reference to the base of a polymorphic object
    {
        std::cout << e.what(); // information from length_error printed
        return {};
    }
    return {};
}