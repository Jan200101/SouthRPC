#ifndef PLUGIN_H
#define PLUGIN_H

#include <vector>

#include "ns_plugin.h"
#include "internal/concommandproxy.h"
#include "internal/convarproxy.h"
#include "internal/sqfuncregistrationproxy.h"

class ServerHandler;

class Plugin {
    private:
        PluginInitFuncs funcs = { 0 };
        PluginNorthstarData data = { 0 };
        PluginEngineData engine_data = { 0 };

        SquirrelFunctions client_sqvm_funcs = { 0 };
        SquirrelFunctions server_sqvm_funcs = { 0 };

        CSquirrelVM client_vm = { 0 };
        CSquirrelVM server_vm = { 0 };
        CSquirrelVM ui_vm = { 0 }; // uses same functions as client

        HMODULE engine = nullptr;
        struct {
            Cbuf_GetCurrentPlayerType Cbuf_GetCurrentPlayer;
            Cbuf_AddTextType Cbuf_AddText;
            Cbuf_ExecuteType Cbuf_Execute;
        } engine_funcs = { 0 };


        std::vector<ConCommandProxy*> commands;
        std::vector<ConVarProxy*> variables;

        std::vector<SQFuncRegistrationProxy*> squirrel_functions;

        void register_server_callbacks();
        HMODULE GetModuleByName(const char* name);

    public:
        ServerHandler* server = nullptr;

        Plugin(PluginInitFuncs* funcs, PluginNorthstarData* data);
        ~Plugin();

        void LoadEngineData(PluginEngineData* data, HMODULE dllPtr);
        void LoadSQVMFunctions(ScriptContext context, SquirrelFunctions* funcs);
        void LoadSQVM(ScriptContext context, CSquirrelVM* sqvm);
        void RemoveSQVM(ScriptContext context);

        void StartServer();
        void RunCommand(const char* cmd);
        SQRESULT RunSquirrelCode(ScriptContext context, std::string code, SQObject* ret_val);
        SQFuncRegistrationProxy* AddNativeSquirrelFunction(std::string returnType, std::string name, std::string argTypes, std::string helpText, ScriptContext context, SQFunction func);

        // Wraps around the internals we receive
        ConCommandProxy* ConCommand(const char* name, PluginFnCommandCallback_t callback, const char* helpString, int flags, void* parent = nullptr);
        ConVarProxy* ConVar(const char* pszName, const char* pszDefaultValue, int nFlags, const char* pszHelpString, bool bMin = 0, float fMin = 0, bool bMax = 0, float fMax = 0, FnChangeCallback_t pCallback = nullptr);
};

#endif