#include "internal/engine/gameconsole.h"

#include "ns_plugin.h"

extern "C" {
	typedef void* (*CreateInterface)(const char *pName, int *pReturnCode);
}

void hook_console_print(HMODULE dllPtr)
{
	assert(dllPtr);

	CreateInterface pCreateInterface = (CreateInterface)GetProcAddress(dllPtr, "CreateInterface");
	assert(pCreateInterface);

	CGameConsole* pGameConsole = (CGameConsole*)pCreateInterface("GameConsole004", NULL);
}
