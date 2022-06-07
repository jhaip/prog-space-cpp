#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include "httplib.h"
#include <nlohmann/json.hpp>
using nlohmann::json;

std::optional<json> do_http_request() {
    try {
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
    } catch (const std::exception &e) // reference to the base of a polymorphic object
    {
        std::cout << e.what(); // information from length_error printed
        return {};
    }
    return {};
}

// from https://github.com/ThePhD/sol2/issues/86
sol::object jsonToLuaObject(const json &j, sol::state_view &lua) {
    if (j.is_null()) {
        return sol::make_object(lua, sol::lua_nil);
    } else if (j.is_boolean()) {
        return sol::make_object(lua, j.get<bool>());
    } else if (j.is_number()) {
        return sol::make_object(lua, j.get<double>());
    } else if (j.is_string()) {
        return sol::make_object(lua, j.get<std::string>().c_str());
    } else if (j.is_object()) {
        sol::table obj = lua.create_table();
        for (auto &el : j.items()) {
            auto thing = jsonToLuaObject(el.value(), lua);
            obj.set(el.key(), thing);
        }
        return obj.as<sol::object>();
    } else if (j.is_array()) {
        sol::table obj = lua.create_table();
        unsigned long i = 0;
        for (auto &el : j.items()) {
            obj.set(i++, jsonToLuaObject(el.value(), lua));
        }
        return obj;
    }
    return sol::make_object(lua, sol::lua_nil);
}

void http_request_thread(std::vector<std::string> query_parts, sol::protected_function callback_func, sol::this_state ts) {
    if (auto response_json = do_http_request()) {
        sol::state_view lua = ts;
        auto r = jsonToLuaObject(response_json.value(), lua);
        callback_func(r);
    }
    std::cout << "making http request == done" << std::endl;
}

void http_request(std::vector<std::string> query_parts, sol::protected_function callback_func, sol::this_state ts) {
    std::cout << "making http request" << std::endl;
    std::thread{http_request_thread, query_parts, callback_func, ts}.detach();
}