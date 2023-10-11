#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <psapi.h>

#include "ns_plugin.h"
#include "plugin.h"
#include "server.h"
#include "internal/types.h"
#include "internal/concommandproxy.h"
#include "internal/convarproxy.h"
#include "internal/sqfuncregistrationproxy.h"

Plugin::Plugin(PluginInitFuncs* funcs, PluginNorthstarData* data)
{
    this->funcs = *funcs;
    this->data = *data;

    this->server = new rpc_server(this);

    spdlog::info(PLUGIN_NAME " initialised!");
}

Plugin::~Plugin()
{
    for (ConCommandProxy* proxy : this->commands)
    {
        delete proxy;
    }
    this->commands.clear();

    for (ConVarProxy* proxy : this->variables)
    {
        delete proxy;
    }
    this->variables.clear();

    server->stop();
    delete server;
}

HMODULE Plugin::GetModuleByName(const char* name)
{
    HMODULE hMods[1024];
    HANDLE hProcess = GetCurrentProcess();
    DWORD cbNeeded;
    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (size_t i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            char szFullModName[MAX_PATH];
            char* szModName = nullptr;

            if (GetModuleFileNameEx(hProcess, hMods[i], szFullModName, sizeof(szFullModName) / sizeof(*szFullModName)))
            {
                szModName = szFullModName + (strlen(szFullModName) - strlen(name));
                if (!strcmp(szModName, name))
                    return hMods[i];
            }
        }
    }

    return nullptr;
}

void Plugin::LoadEngineData(void* data)
{
    this->engine_data = *static_cast<EngineData*>(data);

    PLUGIN_DATA_TYPES plugin_data = {
        &this->funcs,
        &this->data,
        &this->engine_data
    };
    for (ConCommandProxy* proxy : this->commands)
    {
        proxy->initialize(&plugin_data);
    }

    for (ConVarProxy* proxy : this->variables)
    {
        proxy->initialize(&plugin_data);
    }

    this->engine = GetModuleByName("engine.dll");
    if (this->engine)
    {
        // Offsets by Northstar
        // https://github.com/R2Northstar/NorthstarLauncher/blob/0cbdd5672815f956e6b2d2de48d596e87514a07b/NorthstarDLL/engine/r2engine.cpp#L29

        this->engine_funcs.Cbuf_GetCurrentPlayer = reinterpret_cast<Cbuf_GetCurrentPlayerType>(((uintptr_t)this->engine) + 0x120630);
        this->engine_funcs.Cbuf_AddText = reinterpret_cast<Cbuf_AddTextType>(((uintptr_t)this->engine) + 0x1203B0);
        this->engine_funcs.Cbuf_Execute = reinterpret_cast<Cbuf_ExecuteType>(((uintptr_t)this->engine) + 0x1204B0);
    }
}

void Plugin::LoadSQVMFunctions(ScriptContext context, SquirrelFunctions* funcs)
{
    switch (context)
    {
        case ScriptContext::CLIENT:
            this->client_sqvm_funcs = *funcs;
            break;

        case ScriptContext::SERVER:
            this->client_sqvm_funcs = *funcs;
            break;

        case ScriptContext::INVALID:
        default:
            spdlog::warn("Received invalid script context");
            break;
    }
}

void Plugin::LoadSQVM(ScriptContext context, CSquirrelVM* sqvm)
{
    SquirrelFunctions* funcs = nullptr;
    switch (context)
    {
        case ScriptContext::CLIENT:
            this->client_vm = *sqvm;
            funcs = &this->client_sqvm_funcs;
            break;

        case ScriptContext::SERVER:
            this->server_vm = *sqvm;
            funcs = &this->server_sqvm_funcs;
            break;

        case ScriptContext::UI:
            this->ui_vm = *sqvm;
            funcs = &this->client_sqvm_funcs;
            break;

        case ScriptContext::INVALID:
        default:
            spdlog::warn("Received invalid script context");
            return;
    }

    for (SQFuncRegistrationProxy* proxy : this->squirrel_functions)
    {
        if (proxy->getContext() == context)
        {
            SQFuncRegistration* reg = proxy->get();
            spdlog::info("Registering Squirrel Function {}", proxy->getName());
            funcs->RegisterSquirrelFunc(sqvm, reg, 1);
        }
    }
}

void Plugin::RemoveSQVM(ScriptContext context)
{
    switch (context)
    {
        case ScriptContext::CLIENT:
            memset(&this->client_vm, 0, sizeof(CSquirrelVM));
            break;

        case ScriptContext::SERVER:
            memset(&this->server_vm, 0, sizeof(CSquirrelVM));
            break;

        case ScriptContext::UI:
            memset(&this->ui_vm, 0, sizeof(CSquirrelVM));
            break;

        case ScriptContext::INVALID:
        default:
            spdlog::warn("Received invalid script context");
            break;
    }
}

void Plugin::StartServer()
{
    server->start();
}

void Plugin::RunCommand(const char* cmd)
{
    this->engine_funcs.Cbuf_AddText(this->engine_funcs.Cbuf_GetCurrentPlayer(), cmd, cmd_source_t::kCommandSrcCode);
    this->engine_funcs.Cbuf_Execute();
}

SQRESULT Plugin::RunSquirrelCode(ScriptContext context, std::string code, SQObject* ret_val)
{
    CSquirrelVM* sqvm = nullptr;
    SquirrelFunctions* funcs = nullptr;

    switch (context)
    {
        case ScriptContext::CLIENT:
            sqvm = &this->client_vm;
            funcs = &this->client_sqvm_funcs;
            break;

        case ScriptContext::SERVER:
            sqvm = &this->server_vm;
            funcs = &this->server_sqvm_funcs;
            break;

        case ScriptContext::UI:
            sqvm = &this->ui_vm;
            funcs = &this->client_sqvm_funcs;
            break;

        case ScriptContext::INVALID:
        default:
            spdlog::warn("Received invalid script context");
            return SQRESULT_ERROR;
    }

    CompileBufferState bufferState = CompileBufferState(code);
    HSquirrelVM* v = sqvm->sqvm;

    if (SQ_FAILED(funcs->__sq_compilebuffer(v, &bufferState, "rpc", -1, false)))
    {
        spdlog::info("Failed to compile buffer");
        return SQRESULT_ERROR;
    }

    funcs->__sq_pushroottable(v);

    if (SQ_FAILED(funcs->__sq_call(v, 0 + 1, SQTrue, SQFalse)))
    {
        spdlog::info("Failed to execute closure");
        return SQRESULT_ERROR;
    }

    *ret_val = v->_stack[v->_top - 1];

    if (ISREFCOUNTED(ret_val->_Type))
    {
        // pray
        ret_val->_VAL.asString->uiRef++;
    }

    v->_stack[--(v->_top)] = SQ_NULL_OBJ;

    if (ret_val->_Type == OT_NULL)
        return SQRESULT_NULL;

    return SQRESULT_NOTNULL;
}

SQFuncRegistrationProxy* Plugin::AddNativeSquirrelFunction(std::string returnType, std::string name, std::string argTypes, std::string helpText, ScriptContext context, SQFunction func)
{
    SQFuncRegistrationProxy* sqfrp = new SQFuncRegistrationProxy(returnType, name, argTypes, helpText, context, func);

    sqfrp->initialize(nullptr);

    return this->squirrel_functions.emplace_back(sqfrp);
}

ConCommandProxy* Plugin::ConCommand(const char* name, FnCommandCallback_t callback, const char* helpString, int flags, void* parent)
{
    ConCommandProxy* proxy = new ConCommandProxy(name, callback, helpString, flags, parent);

    return this->commands.emplace_back(proxy);
}

ConVarProxy* Plugin::ConVar(const char* pszName, const char* pszDefaultValue, int nFlags, const char* pszHelpString, bool bMin, float fMin, bool bMax, float fMax, FnChangeCallback_t pCallback)
{
    ConVarProxy* proxy = new ConVarProxy(pszName, pszDefaultValue, nFlags, pszHelpString, bMin, fMin, bMax, fMax, pCallback);

    return this->variables.emplace_back(proxy);
}
