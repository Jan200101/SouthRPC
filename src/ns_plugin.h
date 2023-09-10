#ifndef NS_PLUGIN_H
#define NS_PLUGIN_H

// Needed to bootstrap plugin abi
#include <windef.h>
#include <mutex>
#include <optional>
#include <map>
#include <unordered_map>
#include <spdlog/spdlog.h>

#include "plugin_abi.h"
// ConCommand/ConVar types
#include "core/macros.h"
#include "core/convar/convar.h"
#include "core/convar/concommand.h"

// This is a mess
// hope Plugins V3 includes these in the ABI
// pls cat :womp:
typedef void (*ConCommandConstructorType)(ConCommand* newCommand, const char* name, FnCommandCallback_t callback, const char* helpString, int flags, void* parent);
typedef void (*ConVarMallocType)(void* pConVarMaloc, int a2, int a3);
typedef void (*ConVarRegisterType)(ConVar* pConVar, const char* pszName, const char* pszDefaultValue, int nFlags, const char* pszHelpString, bool bMin, float fMin, bool bMax, float fMax, void* pCallback);

#endif
