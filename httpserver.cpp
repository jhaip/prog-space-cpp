#include "Poco/Net/HTTPServer.h"
#include "Poco/Format.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Stringifier.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/WebSocket.h"
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/URI.h>
#include <iostream>
#include <string>
#include <vector>

using namespace Poco::Net;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::WebSocket;
using Poco::Net::WebSocketException;
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

class MyBatchRequestHandler : public HTTPRequestHandler {
  public:
    MyBatchRequestHandler(Database &_db)
        : db(_db) {}

    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp) {
        resp.setStatus(HTTPResponse::HTTP_OK);
        resp.setContentType("application/json");
        resp.set("Access-Control-Allow-Origin", "*");

        auto &stream = req.stream();
        const size_t len = req.getContentLength();
        std::string buffer(len, 0);
        stream.read(buffer.data(), len);

        // [{"type": "retract", "fact": ""}, {"type": "claim", "fact": ""}]
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(buffer);
        Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();
        Poco::JSON::Object::Iterator it;
        {
            std::scoped_lock guard(dbMutex);
            for (Poco::JSON::Array::ConstIterator it = arr->begin(); it != arr->end(); ++it) {
                Poco::JSON::Object::Ptr r = it->extract<Poco::JSON::Object::Ptr>();
                auto type = r->getValue<std::string>("type");
                auto factStr = r->getValue<std::string>("fact");
                if (type == "retract") {
                    std::cout << "making retract: " << factStr << std::endl;
                    db.retract(factStr);
                } else if (type == "claim") {
                    std::cout << "making claim: " << factStr << std::endl;
                    db.claim(factStr);
                }
            }
        }

        ostream &out = resp.send();
        out << "ok";
        out.flush();
    }

  private:
    Database &db;
};

class ErrorPageHandler : public Poco::Net::HTTPRequestHandler {
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

class PageRequestHandler : public HTTPRequestHandler
/// Return a HTML document with some JavaScript creating
/// a WebSocket connection.
{
  public:
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
        response.setChunkedTransferEncoding(true);
        response.setContentType("text/html");
        std::ostream &ostr = response.send();
        ostr << "<html>";
        ostr << "<head>";
        ostr << "<title>WebSocketServer</title>";
        ostr << "<script type=\"text/javascript\">";
        ostr << "function WebSocketTest()";
        ostr << "{";
        ostr << "  if (\"WebSocket\" in window)";
        ostr << "  {";
        ostr << "    var ws = new WebSocket(\"ws://" << request.serverAddress().toString() << "/ws\");";
        ostr << "    ws.onopen = function()";
        ostr << "      {";
        ostr << "        ws.send(\"Hello, world!\");";
        ostr << "      };";
        ostr << "    ws.onmessage = function(evt)";
        ostr << "      { ";
        ostr << "        var msg = evt.data;";
        ostr << "        alert(\"Message received: \" + msg);";
        ostr << "        ws.close();";
        ostr << "      };";
        ostr << "    ws.onclose = function()";
        ostr << "      { ";
        ostr << "        alert(\"WebSocket closed.\");";
        ostr << "      };";
        ostr << "  }";
        ostr << "  else";
        ostr << "  {";
        ostr << "     alert(\"This browser does not support WebSockets.\");";
        ostr << "  }";
        ostr << "}";
        ostr << "</script>";
        ostr << "</head>";
        ostr << "<body>";
        ostr << "  <h1>WebSocket Server</h1>";
        ostr << "  <p><a href=\"javascript:WebSocketTest()\">Run WebSocket Script</a></p>";
        ostr << "</body>";
        ostr << "</html>";
    }
};

class WebSocketRequestHandler : public HTTPRequestHandler
/// Handle a WebSocket connection.
{
  public:
    WebSocketRequestHandler(Database &_db)
        : db(_db) {}

    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
        std::cout << "handling ws req\n";
        // Application &app = Application::instance();
        try {
            std::cout << "trying\n";
            WebSocket ws(request, response);
            std::cout << "WebSocket connection established.\n";
            // app.logger().information("WebSocket connection established.");
            char buffer[1024];
            int flags;
            int n;
            do {
                n = ws.receiveFrame(buffer, sizeof(buffer), flags);
                // app.logger().information(Poco::format("Frame received (length=%d, flags=0x%x).", n, unsigned(flags)));
                if (n > 0) {
                    std::string rcvStr(buffer);
                    // TODO: parse rcvStr into json
                    // TODO: take lock on db so this is safe
                    db.retract("#0ws %");
                    db.claim("#0ws " + rcvStr);
                }
                std::cout << Poco::format("Frame received (length=%d, flags=0x%x).", n, unsigned(flags)) << std::endl;
                ws.sendFrame(buffer, n, flags);
            } while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
            // app.logger().information("WebSocket connection closed.");
            std::cout << "WebSocket connection closed.\n";
        } catch (WebSocketException &exc) {
            std::cout << "websocket execption\n";
            // app.logger().log(exc);
            switch (exc.code()) {
            case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
                response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
                // fallthrough
            case WebSocket::WS_ERR_NO_HANDSHAKE:
            case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
            case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
                response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
                response.setContentLength(0);
                response.send();
                break;
            }
        } catch (const std::exception &e) {
            std::cerr << "ERROR: " << e.what() << std::endl;
        }
    }

  private:
    Database &db;
};

int MyRequestHandler::count = 0;

class MyRequestHandlerFactory : public HTTPRequestHandlerFactory {
  public:
    MyRequestHandlerFactory(Database &_db)
        : db(_db) {}

    virtual HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &req) {
        if (req.find("Upgrade") != req.end() && Poco::icompare(req["Upgrade"], "websocket") == 0) {
            return new WebSocketRequestHandler{db};
        }
        const Poco::URI uri(req.getURI());
        if (uri.getPath() == "/") {
            return new MyRequestHandler{db};
        }
        if (uri.getPath() == "/select") {
            return new MySelectRequestHandler{db};
        }
        if (uri.getPath() == "/batch") {
            return new MyBatchRequestHandler{db};
        }
        if (uri.getPath() == "/wstest") {
            return new PageRequestHandler;
        }
        return new ErrorPageHandler();
    }

  private:
    Database &db;
};