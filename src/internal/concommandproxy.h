#ifndef CONCOMMANDPROXY_H
#define CONCOMMANDPROXY_H

#include "ns_plugin.h"
#include "proxy.h"
#include "types.h"

class ConCommandProxy : public ClassProxy<ConCommand> {
    private:
        const char* name;
        FnCommandCallback_t callback;
        const char* helpString;
        int flags;
        void* parent;

    public:
        ConCommandProxy(
            const char* name,
            FnCommandCallback_t callback,
            const char* helpString,
            int flags,
            void* parent
        ) :
            name(name),
            callback(callback),
            helpString(helpString),
            flags(flags),
            parent(parent)
        {}

        virtual void initialize(void* data)
        {
            PLUGIN_DATA_TYPES* plugin_data = static_cast<PLUGIN_DATA_TYPES*>(data);

            PluginInitFuncs* funcs = plugin_data->funcs;
            EngineData* engine_data = plugin_data->engine_data;

            assert(funcs->createObject);
            assert(engine_data->ConCommandConstructor);

            ptr = static_cast<ConCommand*>(funcs->createObject(ObjectType::CONCOMMANDS));

            spdlog::info("Registering ConCommand {}", name);
            engine_data->ConCommandConstructor(ptr, name, callback, helpString, flags, parent);
        }
};

#endif