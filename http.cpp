#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
#include "httplib.h"
#include <nlohmann/json.hpp>
#include "Poco/URI.h"
using nlohmann::json;

std::optional<json> do_http_request(std::string url) {
    try {
        Poco::URI uri1(url);
        std::string server = uri1.getScheme() + "://" + uri1.getAuthority();
        std::string path = uri1.getPathEtc();
        httplib::Client cli(server.c_str());
        cli.enable_server_certificate_verification(false);

        auto res = cli.Get(path.c_str());
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

void http_request_thread_v2(std::string url, std::string requestId, Database &db) {
    if (auto response_json = do_http_request(url)) {
        std::scoped_lock guard(dbMutex);
        std::string responseStr = response_json.value().dump();
        db.retract("#0httpreq http result id "+requestId+" %");
        db.claim(Fact{{Term{"#0httpreq"}, Term{"http"}, Term{"result"}, Term{"id"}, Term{requestId},
                       Term{"", responseStr}}});
        std::cout << "claimed http result " << requestId << std::endl;
    }
    std::cout << "making http request == done" << std::endl;
}

class HTTPCallManager {
  public:
    void update(Database &db) {
        auto results = db.select({"$ wish http request to $url with id $requestId"});
        if (results.size() > 0) {
            for (const auto &result : results) {
                std::string url = result.get("url").value;
                std::string requestId = result.get("requestId").value;
                std::thread{http_request_thread_v2, url, requestId, std::ref(db)}.detach();
            }
            db.retract("$ wish http request to $ with id $");
        }
    }
};