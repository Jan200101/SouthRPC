#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>

#include "rapidjson/error/en.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/allocators.h"

#include "ns_plugin.h"
#include "internal/convarproxy.h"

#define SLEEP_DURATION 5000
#define DEFAULT_PORT "26503"

#define METHOD_POST "POST"

#define REQUEST_SIZE 4096
#define HTTP_LF "\r\n"
#define BODY_SEP HTTP_LF HTTP_LF 
#define RESP(STATUS, TYPE, BODY) "HTTP/1.1 " STATUS HTTP_LF  "Content-Type: " TYPE BODY_SEP BODY 

#define RESP_200(TYPE, BODY) RESP("200 OK", TYPE, BODY)
#define RESP_400(TYPE, BODY) RESP("400 Bad Request", TYPE, BODY)
#define RESP_404(TYPE, BODY) RESP("404 Not Found", TYPE, BODY)

#define RESP_OK RESP_200("text/plain", "")
#define RESP_JSON RESP_200("application/json", "")
#define RESP_FAIL_PARSE RESP_400("text/plain", "Failed to parse request")

#define RESP_RPC_PARAMS_ARR RESP_400("text/plain", "SouthRPC cannot handle parameters in an array")
#define RESP_RPC_INVALID_METHOD RESP_400("text/plain", "Invalid RPC Method")
#define RESP_RPC_MISSING_PARAM RESP_400("text/plain", "Missing RPC Parameter")

#define RESP_INVALID_RPC RESP_400("text/plain", "Request is invalid JSON-RPC 2.0")
#define RESP_INVALID_METHOD RESP_400("text/plain", "Invalid HTTP Method")
#define RESP_INVALID_ENDPOINT RESP_404("text/plain", "Invalid Endpoint")

#define RESP_SQUIRREL_ERROR RESP_400("text/plain", "Failed to execute squirrel code")
#define RESP_SQUIRREL_INVALID_CONTEXT RESP_400("text/plain", "Invalid Squirrel Context")


class Plugin;

class rpc_server {
    private:
        Plugin* parent;
        bool initialized = false;

        WSADATA wsaData;
        ConVarProxy* Convar_Port = nullptr;
        ConVarProxy* Convar_Connections = nullptr;

        bool running = false;
        HANDLE thread = nullptr;

        void SquirrelToJSON(
            rapidjson::Value* out_val,
            rapidjson::MemoryPoolAllocator<>& allocator,
            SQObject* obj_ptr
        );

    public:
        rpc_server(Plugin* plugin);
        ~rpc_server();

        void start();
        void stop();
        DWORD run();
};

#endif