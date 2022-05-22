#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>
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

int MyRequestHandler::count = 0;

class MyRequestHandlerFactory : public HTTPRequestHandlerFactory {
  public:
    MyRequestHandlerFactory(Database &_db)
        : db(_db) {}

    virtual HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &) {
        return new MyRequestHandler{db};
    }

  private:
    Database &db;
};