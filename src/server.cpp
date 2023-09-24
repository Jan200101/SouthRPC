#include <winsock2.h>
#include <ws2tcpip.h>

#include <spdlog/spdlog.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "server.h"
#include "plugin.h"

ConVar* Cvar_southrpc_port;

void ConCommand_southrpc_status(const CCommand& args)
{
    spdlog::info("Yes");
}

rpc_server::rpc_server(Plugin* plugin)
    : parent(plugin)
{
    plugin->ConCommand("southrpc_status", ConCommand_southrpc_status, "", 0);

    this->Convar_Port = plugin->ConVar("southrpc_port", DEFAULT_PORT, FCVAR_ARCHIVE, "South RPC HTTP Port (requires restart, default: " DEFAULT_PORT ")");

    if (WSAStartup(MAKEWORD(2, 2), &this->wsaData) != 0) {
        spdlog::error("Failed to open Windows Socket");
        return;
    }

    if (LOBYTE(this->wsaData.wVersion) != 2 ||
        HIBYTE(this->wsaData.wVersion) != 2)
    {
        spdlog::error("Incorrect Winsock version loaded");
        WSACleanup();
        return;
    }

    initialized = true;
}

rpc_server::~rpc_server()
{
    this->stop();
}

static DWORD WINAPI static_server_run(void* param)
{
    rpc_server* server = (rpc_server*)param;

    return server->run();
}

void rpc_server::start()
{
    if (!initialized)
    {
        return;
    }

    this->running = true;

    DWORD thread_id;
    this->thread = CreateThread(NULL, 0, static_server_run, (void*)this, 0, &thread_id);
}

void rpc_server::stop()
{
    if (!this->thread)
        return;

    this->running = false;
    this->thread = nullptr;
}

DWORD rpc_server::run()
{
    // Waiting for engine to init so we can actually use convar values from the archive
    Sleep(SLEEP_DURATION);

    int port = this->Convar_Port->GetInt();

    struct sockaddr_in local = { 0 };
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(port);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        spdlog::error("Failed to create socket");
        goto run_error;
    }

    if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
    {
        spdlog::error("Failed to bindsocket");
        goto run_error;
    }

    if (listen(sock, 10) == SOCKET_ERROR)
    {
        spdlog::error("Failed to listen to socket");
        goto run_error;
    }

    spdlog::info("Running under {}", port);

    SOCKET msg;
    struct sockaddr_in addr;
    int addr_len;

    char buf[REQUEST_SIZE];
    char http_method[10];
    char protocol[10];

    this->running = true;
    while (this->running)
    {
        addr_len = sizeof(addr);
        msg = accept(sock, (struct sockaddr*)&addr, &addr_len);
        if (msg == INVALID_SOCKET || msg == -1)
        {
            spdlog::error("Received invalid request ({})", WSAGetLastError());
            continue;
        };

        spdlog::info("Connection opened by {}", inet_ntoa(addr.sin_addr));

        memset(buf, 0, sizeof(buf));
        if (recv(msg, buf, sizeof(buf), 0) == SOCKET_ERROR)
        {
            spdlog::error("Received no data ({})", WSAGetLastError());
            closesocket(msg);
            continue;
        }

        buf[REQUEST_SIZE - 1] = '\0';

        memset(http_method, 0, sizeof(http_method));
        if (sscanf(buf, "%s /rpc %s" HTTP_LF, &http_method, &protocol) < 2)
        {
            spdlog::error("Request is not to the correct endpoint");
            send(msg, RESP_INVALID_ENDPOINT, strlen(RESP_INVALID_ENDPOINT), 0);
            closesocket(msg);
            continue;
        }

        if (strncmp(http_method, METHOD_POST, strlen(METHOD_POST)+1))
        {
            spdlog::error("Request is not a " METHOD_POST " request");
            send(msg, RESP_INVALID_METHOD, strlen(RESP_INVALID_METHOD), 0);
            closesocket(msg);
            continue;
        }

        char* body = nullptr;
        char* ptr = buf;
        while (*ptr)
        {
            if (!memcmp(ptr, BODY_SEP, 4))
            {
                body = ptr + 4;
                break;
            }
            ++ptr;
        }

        if (!body || !*body)
        {
            spdlog::error("No request body found");
            send(msg, RESP_INVALID_RPC, strlen(RESP_INVALID_RPC), 0);
            closesocket(msg);
            continue;
        }

        rapidjson::Document json_body;
        json_body.Parse(body);

        if (json_body.HasParseError())
        {
            spdlog::error("Failed to parse request");
            send(msg, RESP_FAIL_PARSE, strlen(RESP_FAIL_PARSE), 0);
        }
        else if (!json_body.IsObject() ||
            !(json_body.HasMember("jsonrpc") && json_body["jsonrpc"] == "2.0") ||
            !(json_body.HasMember("id") && (json_body["id"].IsInt() || json_body["id"].IsNull())) ||
            !(json_body.HasMember("method") && json_body["method"].IsString()) ||
            (json_body.HasMember("params") && !(json_body["params"].IsObject() )))
        {
            spdlog::error("Request is not valid JSON-RPC 2.0");
            send(msg, RESP_INVALID_RPC, strlen(RESP_INVALID_RPC), 0);
        }
        else
        {
            const char* method = json_body["method"].GetString();
            auto params = json_body["params"].GetObject();

            spdlog::info("Received request for method \"{}\"", method);

            if (!strcmp(method, "execute_command"))
            {
                if (!params.HasMember("command"))
                {
                    send(msg, RESP_RPC_MISSING_PARAM, strlen(RESP_RPC_MISSING_PARAM), 0);
                    closesocket(msg);
                    continue;
                }

                const char* cmd = params["command"].GetString();
                this->parent->RunCommand(cmd);

                send(msg, RESP_JSON, strlen(RESP_JSON), 0);
                if (!json_body["id"].IsNull())
                {
                    rapidjson::Document doc;
                    rapidjson::MemoryPoolAllocator<>& allocator = doc.GetAllocator();
                    doc.SetObject();

                    rapidjson::Value result;

                    doc.AddMember("jsonrpc", "2.0", allocator);
                    doc.AddMember("id", json_body["id"], allocator);
                    doc.AddMember("result", result, allocator);

                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

                    doc.Accept(writer);

                    const char* json_str = buffer.GetString();

                    send(msg, json_str, strlen(json_str), 0);
                }
            }
            else if (!strcmp(method, "execute_squirrel"))
            {
                if (!params.HasMember("code"))
                {
                    send(msg, RESP_RPC_MISSING_PARAM, strlen(RESP_RPC_MISSING_PARAM), 0);
                    closesocket(msg);
                    continue;
                }

                const char* code = params["code"].GetString();
                ScriptContext context = ScriptContext::UI;

                if (params.HasMember("context"))
                {
                    if (params["context"] == "client")
                    {
                        context = ScriptContext::CLIENT;
                    }
                    else if (params["context"] == "server")
                    {
                        context = ScriptContext::SERVER;
                    }
                    else if (params["context"] == "ui")
                    {
                        context = ScriptContext::UI;
                    }
                    else
                    {
                        send(msg, RESP_SQUIRREL_INVALID_CONTEXT, strlen(RESP_SQUIRREL_INVALID_CONTEXT), 0);
                        closesocket(msg);
                        continue;
                    }
                }
                SQObject obj_ptr;

                this->parent->RunSquirrelCode(context, code, &obj_ptr);

                send(msg, RESP_JSON, strlen(RESP_JSON), 0);

                if (!json_body["id"].IsNull())
                {
                    rapidjson::Document doc;
                    rapidjson::MemoryPoolAllocator<>& allocator = doc.GetAllocator();
                    doc.SetObject();

                    rapidjson::Value result;
                    SquirrelToJSON(&result, allocator, &obj_ptr);

                    doc.AddMember("jsonrpc", "2.0", allocator);
                    doc.AddMember("id", json_body["id"], allocator);
                    doc.AddMember("result", result, allocator);

                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

                    doc.Accept(writer);

                    const char* json_str = buffer.GetString();

                    send(msg, json_str, strlen(json_str), 0);
                }

                if (ISREFCOUNTED(obj_ptr._Type))
                {
                    // pray
                    obj_ptr._VAL.asString->uiRef--;
                }
            }
            else
            {
                send(msg, RESP_RPC_INVALID_METHOD, strlen(RESP_RPC_INVALID_METHOD), 0);
            }
 
        }

        closesocket(msg);
    }

run_error:
    this->running = false;

    return 0;
}

void rpc_server::SquirrelToJSON(
    rapidjson::Value* out_val,
    rapidjson::MemoryPoolAllocator<>& allocator,
    SQObject* obj_ptr
) {

    if (obj_ptr)
    {
        switch (obj_ptr->_Type)
        {
            case OT_BOOL:
                out_val->SetBool(obj_ptr->_VAL.asInteger);
                break;

            case OT_INTEGER:
                out_val->SetInt(obj_ptr->_VAL.asInteger);
                break;

            case OT_STRING:
                out_val->SetString(obj_ptr->_VAL.asString->_val, obj_ptr->_VAL.asString->length);
                break;

            case OT_ARRAY:
                out_val->SetArray();

                for (int i = 0; i < obj_ptr->_VAL.asArray->_usedSlots; ++i)
                {
                    rapidjson::Value n;
                    SquirrelToJSON(&n, allocator, &obj_ptr->_VAL.asArray->_values[i]);
                    out_val->PushBack(n, allocator);
                }
                break;

            case OT_TABLE:
                out_val->SetObject();

                for (int i = 0; i < obj_ptr->_VAL.asTable->_numOfNodes; ++i)
                {
                    auto node = &obj_ptr->_VAL.asTable->_nodes[i];
                    if (node->key._Type != OT_STRING)
                        continue;

                    rapidjson::Value k;
                    SquirrelToJSON(&k, allocator, &node->key);

                    rapidjson::Value v;
                    SquirrelToJSON(&v, allocator, &node->val);
                    out_val->AddMember(k, v, allocator);
                }
                break;

            default:
                break;
        }
    }
}
