#ifndef PLUGIN_H
#define PLUGIN_H

#include "ns_plugin.h"

class jsonrpc_server;

struct EngineData
{
    ConCommandConstructorType ConCommandConstructor;
    ConVarMallocType conVarMalloc;
    ConVarRegisterType conVarRegister;
    void* ConVar_Vtable;
    void* IConVar_Vtable;
};

class Plugin {
    private:
        PluginInitFuncs* funcs = nullptr;
        PluginNorthstarData* data = nullptr;
        EngineData* engine_data = nullptr;

        jsonrpc_server* server;

    public:
        Plugin(PluginInitFuncs* funcs, PluginNorthstarData* data);
        ~Plugin();

        void LoadEngineData(void* data);
        void RegisterConCommand(const char* name, FnCommandCallback_t callback, const char* helpString, int flags);
};

#endif