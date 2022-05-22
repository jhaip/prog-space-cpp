#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/URI.h>
#include <iostream>
#include <string>
#include <vector>

using namespace Poco::Net;
using namespace std;

class MyRequestHandler : public HTTPRequestHandler {
  public:
    MyRequestHandler(Database &_db)
        : db(_db) {}

    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp) {
        resp.setStatus(HTTPResponse::HTTP_OK);
        resp.setContentType("text/html");
        resp.set("Access-Control-Allow-Origin", "*");

        ostream &out = resp.send();
        out << "<h1>Hello world!</h1>"
            << "<p>Count: " << ++count << "</p>"
            << "<p>Host: " << req.getHost() << "</p>"
            << "<p>Method: " << req.getMethod() << "</p>"
            << "<p>URI: " << req.getURI() << "</p>";
        out.flush();

        cout << endl
             << "Response sent for count=" << count
             << " and URI=" << req.getURI() << endl;

        std::scoped_lock guard(dbMutex);
        db.retract("#999 %");
        db.claim("#999 count is " + std::to_string(count));
    }

  private:
    static int count;
    Database &db;
};

class MySelectRequestHandler : public HTTPRequestHandler {
  public:
    MySelectRequestHandler(Database &_db)
        : db(_db) {}

    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp) {
        resp.setStatus(HTTPResponse::HTTP_OK);
        resp.setContentType("application/json");
        resp.set("Access-Control-Allow-Origin", "*");

        const Poco::URI Uri(req.getURI());
        const Poco::URI::QueryParameters qps = Uri.getQueryParameters();

        std::string dbQueryStr;
        bool returnFirstResult = false;
        cout << "Query params:" << endl;
        for (const auto &qp : qps) {
            cout << qp.first << ": " << qp.second << endl;
            if (qp.first == "query") {
                dbQueryStr = qp.second;
            } else if (qp.first == "first") {
                returnFirstResult = true;
            }
        }
        if (dbQueryStr.length() == 0) {
            resp.setContentType("text/html");
            // Sets the response status 404, 200 etc.
            resp.setStatus("400");
            std::ostream &responseStream = resp.send();
            responseStream << "missing query";
        } else {
            std::vector<std::string> query_parts;
            Poco::JSON::Parser parser;
            Poco::Dynamic::Var result = parser.parse(dbQueryStr);
            Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();
            Poco::JSON::Object::Iterator it;
            for (Poco::JSON::Array::ConstIterator it = arr->begin(); it != arr->end(); ++it) {
                query_parts.push_back(it->convert<std::string>());
            }
            std::vector<QueryResult> queryResults;
            {
                std::scoped_lock guard(dbMutex);
                queryResults = db.select(query_parts);
            }
            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
            Poco::JSON::Array::Ptr resultsArray = new Poco::JSON::Array();
            int resultsArrayIndex = 0;
            for (const auto &queryResult : queryResults) {
                Poco::JSON::Object::Ptr arrObj = new Poco::JSON::Object();
                for (const auto &result : queryResult.Result) {
                    Poco::JSON::Array::Ptr termArray = new Poco::JSON::Array();
                    termArray->set(0, result.second.type);
                    termArray->set(1, result.second.value);
                    arrObj->set(result.first, termArray);
                }
                resultsArray->set(resultsArrayIndex++, arrObj);
            }
            root->set("results", resultsArray);
            ostream &out = resp.send();
            if (returnFirstResult && resultsArrayIndex > 0) {
                resultsArray->get(0).extract<Poco::JSON::Object::Ptr>()->stringify(out);
            } else {
                resultsArray->stringify(out);
            }
            // out << "{\"results\": \"ok\"}";
            out.flush();

            cout << endl
                 << " and URI=" << req.getURI() << endl;
        }
    }

  private:
    Database &db;
};

class ErroPageHandler : public Poco::Net::HTTPRequestHandler {
  public:
    void handleRequest(Poco::Net::HTTPServerRequest &request, Poco::Net::HTTPServerResponse &response) {
        response.setChunkedTransferEncoding(true);
        // Sets mime type text/html application/json etc.
        response.setContentType("text/html");
        // Sets the response status 404, 200 etc.
        response.setStatus("404");

        // opens the file stream
        std::ostream &responseStream = response.send();

        responseStream << "<html><head><head><title>My  HTTP Server in C++ </title></head>";
        responseStream << "<body><h1>PAGE NOT FOUND, SORRY!</h1><p>";
        responseStream << "";
        responseStream << "</p></body></html>";
    };
};

int MyRequestHandler::count = 0;

class MyRequestHandlerFactory : public HTTPRequestHandlerFactory {
  public:
    MyRequestHandlerFactory(Database &_db)
        : db(_db) {}

    virtual HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &req) {
        const Poco::URI uri(req.getURI());
        if (uri.getPath() == "/") {
            return new MyRequestHandler{db};
        }
        if (uri.getPath() == "/select") {
            return new MySelectRequestHandler{db};
        }
        return new ErroPageHandler();
    }

  private:
    Database &db;
};