#include <winsock2.h>

#include "handler.h"
#include "plugin.h"
#include "internal/convarproxy.h"

#define SLEEP_DURATION 5000

ServerHandler::ServerHandler(Plugin* plugin):
	plugin(plugin)
{
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

    spdlog::info("Initialized handler");

    this->init = true;

    /* execute_squirrel */
    this->register_callback(
        "methods",
        [this](rapidjson::MemoryPoolAllocator<>& allocator, rapidjson::Value& params) -> rapidjson::Value
        {
            rapidjson::Value method_list;
            method_list.SetArray();

            for (auto const& [key, val] : this->methods)
            {
                rapidjson::Value method_name;
                method_name.SetString(key.c_str(), key.size(), allocator);

                method_list.PushBack(method_name, allocator);
            }

            return method_list;
        }
    );

}

static DWORD WINAPI static_server_run(void* param)
{
    ServerHandler* server = (ServerHandler*)param;

    server->run();

    return 0;
}

void ServerHandler::start()
{
	if (!this->init || this->running)
		return;

    DWORD thread_id;
    this->thread = CreateThread(NULL, 0, static_server_run, (void*)this, 0, &thread_id);
}

void ServerHandler::stop()
{
    if (!this->thread)
        return;

    this->running = false;
    TerminateThread(this->thread, 0);
    this->thread = nullptr;
}

void ServerHandler::run()
{
	this->running = true;

    spdlog::info("Running Handler");

    // The engine won't have loaded convar data until after the entry
    spdlog::info("Waiting for engine to initialize");
    Sleep(SLEEP_DURATION);

    int port = this->Convar_Port->GetInt();
    RPCServer* server = new RPCServer(INADDR_ANY, port);

    if (server->get_http_server().get_socket() == -1)
    {
        spdlog::error("HTTP Server failed to start");
        return;
    }

    spdlog::info("Launched server on port {}", port);

    while (this->running)
    {
    	RPCRequest* request = server->receive_request();
        spdlog::debug("received request");
    	if (!request)
    		continue;

        std::string method_name = request->get_method();

        callback_list::const_iterator method_pos = this->methods.find(method_name);
        if (method_pos != this->methods.end())
        {
            spdlog::info("Invoked method '{}'", method_name);
            request->result(method_pos->second(request->get_allocator(), request->get_params()));
        }
        else
        {
            spdlog::error("Attempted to invoke unknown method '{}'", method_name);
            request->error(rapidjson::Value("Unknown method"));
        }

        delete request;
    }

    delete server;
}
