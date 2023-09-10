#include "ns_plugin.h"
#include "plugin.h"
#include "server.h"

Plugin::Plugin(PluginInitFuncs* funcs, PluginNorthstarData* data)
	: server(new jsonrpc_server(this))
{
	this->funcs = funcs;
	this->data = data;

	spdlog::info(PLUGIN_NAME " initialised!");
}

Plugin::~Plugin()
{
	delete server;
}

void Plugin::LoadEngineData(void* data)
{
	this->engine_data = static_cast<EngineData*>(data);

	spdlog::info("Engine data loaded");
}