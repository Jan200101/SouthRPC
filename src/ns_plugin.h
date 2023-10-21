#ifndef NS_PLUGIN_H
#define NS_PLUGIN_H

#define WIN32_LEAN_AND_MEAN
// Needed for RapidJSON to function
#define RAPIDJSON_NOMEMBERITERATORCLASS
#define NOMINMAX
#define RAPIDJSON_HAS_STDSTRING 1

// Needed to bootstrap plugin abi
#include <windows.h>
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

//#include "engine/r2engine.h"
// Import r2engine isn't possible, vendor stuff
enum class ECommandTarget_t
{
    CBUF_FIRST_PLAYER = 0,
    CBUF_LAST_PLAYER = 1, // MAX_SPLITSCREEN_CLIENTS - 1, MAX_SPLITSCREEN_CLIENTS = 2
    CBUF_SERVER = CBUF_LAST_PLAYER + 1,

    CBUF_COUNT,
};

enum class cmd_source_t
{
    // Added to the console buffer by gameplay code.  Generally unrestricted.
    kCommandSrcCode,

    // Sent from code via engine->ClientCmd, which is restricted to commands visible
    // via FCVAR_GAMEDLL_FOR_REMOTE_CLIENTS.
    kCommandSrcClientCmd,

    // Typed in at the console or via a user key-bind.  Generally unrestricted, although
    // the client will throttle commands sent to the server this way to 16 per second.
    kCommandSrcUserInput,

    // Came in over a net connection as a clc_stringcmd
    // host_client will be valid during this state.
    //
    // Restricted to FCVAR_GAMEDLL commands (but not convars) and special non-ConCommand
    // server commands hardcoded into gameplay code (e.g. "joingame")
    kCommandSrcNetClient,

    // Received from the server as the client
    //
    // Restricted to commands with FCVAR_SERVER_CAN_EXECUTE
    kCommandSrcNetServer,

    // Being played back from a demo file
    //
    // Not currently restricted by convar flag, but some commands manually ignore calls
    // from this source.  FIXME: Should be heavily restricted as demo commands can come
    // from untrusted sources.
    kCommandSrcDemoFile,

    // Invalid value used when cleared
    kCommandSrcInvalid = -1
};

typedef ECommandTarget_t (*Cbuf_GetCurrentPlayerType)();
typedef void (*Cbuf_AddTextType)(ECommandTarget_t eTarget, const char* text, cmd_source_t source);
typedef void (*Cbuf_ExecuteType)();

#endif
