#include <winsock2.h>
#include <ws2tcpip.h>

#include <spdlog/spdlog.h>

#include "server.h"
#include "plugin.h"

jsonrpc_server::jsonrpc_server(Plugin* plugin)
{
    this->parent = plugin; 
    spdlog::info("jsonrpc_server::jsonrpc_server()");
}

jsonrpc_server::~jsonrpc_server()
{
    spdlog::info("jsonrpc_server::~jsonrpc_server()");
}