#ifndef RPC_SERVER
#define RPC_SERVER

#include <map>
#include <string>

#include <rapidjson/document.h>

#include "http_server.h"

class RPCRequest {
    private:
        bool valid = false;
        HTTPRequest* request;

        rapidjson::Document body;

        rapidjson::Value id;
        std::string method;
        rapidjson::Value params;

        void response(rapidjson::Value& result);

    public:
        RPCRequest(HTTPRequest* request);
        ~RPCRequest();

        bool is_valid() { return this->valid; };

        rapidjson::MemoryPoolAllocator<>& get_allocator() { return this->body.GetAllocator(); };
        std::string get_method() { return this->method; }
        rapidjson::Value& get_params() { return this->params; }

        void result(rapidjson::Value result);
        void error(rapidjson::Value error);
        void close();
};

class RPCServer {
    private:
        HTTPServer http_server;

    public:
        RPCServer(unsigned long addr, unsigned short port);

        HTTPServer& get_http_server() { return this->http_server; }

        RPCRequest* receive_request();
};

#endif
