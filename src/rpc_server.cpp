
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "rpc_server.h"

#define RPC_PREFIX "rpc."

RPCRequest::RPCRequest(HTTPRequest* request):
    request(request)
{
    if (!request)
        return;

    this->body.Parse(request->get_body());

    if (!this->body.HasMember("jsonrpc"))
    {
        spdlog::error("Request is missing 'jsonrpc' field");
        this->error(rapidjson::Value("Request is missing 'jsonrpc' field"));
        return;
    }
    else if (this->body["jsonrpc"] != "2.0")
    {
        spdlog::error("Request has invalid value for 'jsonrpc' field");
        this->error(rapidjson::Value("Request has invalid value for 'jsonrpc' field"));
        return;
    }

    if (this->body.HasMember("id"))
    {
        this->id = this->body["id"];

        if (!this->id.IsInt() && !this->id.IsString() && !this->id.IsNull())
        {
            spdlog::error("Request has invalid type for 'id' field");
            this->error(rapidjson::Value("Request has invalid type for 'id' field"));
            return;
        }
    }

    if (!this->body.HasMember("method"))
    {
        spdlog::error("Request is missing 'method' field");
        this->error(rapidjson::Value("Request is missing 'method' field"));
        return;
    }
    else if (!this->body["method"].IsString())
    {
        spdlog::error("Request has invalid type for 'method' field");
        this->error(rapidjson::Value("Request has invalid type for 'method' field"));
        return;
    }

    this->method = this->body["method"].GetString();

    if (!strncmp(this->method.c_str(), RPC_PREFIX, strlen(RPC_PREFIX)))
    {
        spdlog::error("Request uses reserved value for 'method' field");
        this->error(rapidjson::Value("Request uses reserved value for 'method' field"));
        return;
    }

    if (this->body.HasMember("params"))
    {
        this->params = this->body["params"];

        if (!this->params.IsArray() && !this->params.IsObject())
        {
            spdlog::error("Request has invalid type for 'params' field");
            this->error(rapidjson::Value("Request has invalid type for 'params' field"));
            return;
        }
    }

    this->valid = true;
    spdlog::debug("Valid request");
}

RPCRequest::~RPCRequest()
{
    delete this->request;
}

void RPCRequest::response(rapidjson::Value& response)
{
    // a request without an ID is a Notification.
    // Nothing we can do about it
    if (!this->body.HasMember("id"))
        return;

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

    response.Accept(writer);

    this->request->respond(HTTP_OK, {
        {"Content-Type", "application/json"}
    }, buffer.GetString());
}

void RPCRequest::result(rapidjson::Value result)
{
    if (this->valid && !this->body.HasMember("id"))
        return;

    rapidjson::MemoryPoolAllocator<>& allocator = this->body.GetAllocator();

    rapidjson::Value error_resp;
    error_resp.SetObject();

    error_resp.AddMember("jsonrpc", "2.0", allocator);
    error_resp.AddMember("id", this->id, allocator);
    error_resp.AddMember("result", result, allocator);

    this->response(error_resp);
}

void RPCRequest::error(rapidjson::Value error)
{
    if (!this->body.HasMember("id"))
        return;

    rapidjson::MemoryPoolAllocator<>& allocator = this->body.GetAllocator();

    rapidjson::Value error_resp;
    error_resp.SetObject();

    error_resp.AddMember("jsonrpc", "2.0", allocator);
    error_resp.AddMember("id", this->id, allocator);
    error_resp.AddMember("error", error, allocator);

    this->response(error_resp);
}

void RPCRequest::close()
{
    if (this->request)
    {
        this->request->close();
    }
}

RPCServer::RPCServer(unsigned long addr, unsigned short port):
    http_server(addr, port)
{
}

RPCRequest* RPCServer::receive_request()
{
    spdlog::debug("awaiting JSON-RPC request");

    HTTPRequest* http_req = this->http_server.receive_request();
    if (!http_req)
        return nullptr;

    RPCRequest* rpc_req = new RPCRequest(http_req);

    return rpc_req;
}