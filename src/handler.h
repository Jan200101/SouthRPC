#ifndef HANDLER_H
#define HANDLER_H

#include <functional>

#include <rapidjson/document.h>

#include "rpc_server.h"
#include "plugin.h"
#include "internal/convarproxy.h"

#define DEFAULT_PORT "26503"

typedef std::function<rapidjson::Value(rapidjson::MemoryPoolAllocator<>&, rapidjson::Value&)> method_callback;
typedef std::map<std::string, method_callback> callback_list;

class ServerHandler
{
    private:
        bool init = false;
        Plugin* plugin;

        WSADATA wsaData;

        bool running = false;
        HANDLE thread = nullptr;

        ConVarProxy* Convar_Port = nullptr;

        callback_list methods;
    public:
        ServerHandler(Plugin* parent);

        void register_callback(std::string name, method_callback callback) { this->methods.try_emplace(name, callback); }

        void start();
        void stop();
        void run();
};

#endif
