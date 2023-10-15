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

    /*
    @TODO
    this->register_callback(
        "list_methods",
        [=](rapidjson::Value& params) -> rapidjson::Value
        {
            rapidjson::MemoryPoolAllocator<>& allocator = this->body.GetAllocator();

            return rapidjson::Value(1);
        }
    );
    */

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
    //Sleep(SLEEP_DURATION);

    //int port = this->Convar_Port->GetInt();
    int port = 26503;
    RPCServer* server = new RPCServer(INADDR_ANY, port);
    Sleep(SLEEP_DURATION);

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
