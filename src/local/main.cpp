#include "ns_plugin.h"
#include "plugin.h"
#include "handler.h"

int main()
{
	spdlog::info("Main");

	Plugin plugin(nullptr, nullptr);

	plugin.server->run();
}