#include "ns_plugin.h"
#include "internal/logging.h"

#include "server.h"
#include "plugin.h"

Plugin* plugin = nullptr;

extern "C" __declspec(dllexport)
void PLUGIN_INIT(PluginInitFuncs* funcs, PluginNorthstarData* data)
{
    spdlog::default_logger()->sinks().pop_back();
    spdlog::default_logger()->sinks().push_back(std::make_shared<PluginSink>(funcs->logger));

    plugin = new Plugin(funcs, data);
}

extern "C" __declspec(dllexport)
void PLUGIN_DEINIT()
{
    if (plugin)
    {
        delete plugin;
        plugin = nullptr;
    }
}

extern "C" __declspec(dllexport)
void PLUGIN_INFORM_DLL_LOAD(PluginLoadDLL dll, void* data) {
    switch (dll) {
        case PluginLoadDLL::ENGINE:
            plugin->LoadEngineData(data);
            break;
        case PluginLoadDLL::CLIENT:
        case PluginLoadDLL::SERVER:
            break;
        default:
            spdlog::warn("PLUGIN_INFORM_DLL_LOAD called with unknown type {}", (int)dll);
            break;
    }
}

// There is no deinit logic for Plugins
// Recreate it using DllMain
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_DETACH:
        PLUGIN_DEINIT();

    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
