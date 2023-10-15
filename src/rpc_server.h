#ifndef RPC_SERVER
#define RPC_SERVER

#include <map>
#include <string>

#include <rapidjson/document.h>

#include "http_server.h"

class RPCRequest {
    private:
        HTTPRequest* request;

        rapidjson::Document body;

        rapidjson::Value id;
        std::string method;
        rapidjson::Value params;

        void response(rapidjson::Value& result);

    public:
        RPCRequest(HTTPRequest* request);
        ~RPCRequest();

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

        RPCRequest* receive_request();
};

#endif
