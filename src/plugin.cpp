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

	this->RegisterConCommand("south_test", [](const CCommand& command){ spdlog::info("Gaming"); }, "", 0);
}

void Plugin::RegisterConCommand(const char* name, FnCommandCallback_t callback, const char* helpString, int flags)
{
	if (!this->engine_data)
	{
		return;
	}

	spdlog::info("Registering ConCommand {}", name);

	extern_CreateObjectFunc createObject = static_cast<extern_CreateObjectFunc>(this->funcs->createObject); 

	spdlog::info("Creating Object");
	void* command = createObject(ObjectType::CONCOMMANDS);

	spdlog::info("Constructing Command");
	this->engine_data->ConCommandConstructor((ConCommand*)command, name, callback, helpString, flags, nullptr);
}
