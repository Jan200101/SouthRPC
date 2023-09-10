// Needed to bootstrap plugin abi
#include "plugin.h"
#include "internal/logging.h"

extern "C" __declspec(dllexport)
void PLUGIN_INIT(PluginInitFuncs* funcs, PluginNorthstarData* data)
{
    spdlog::default_logger()->sinks().pop_back();
    spdlog::default_logger()->sinks().push_back(std::make_shared<PluginSink>(funcs->logger));

    spdlog::info(PLUGIN_NAME " succesfully initialised!");
}
