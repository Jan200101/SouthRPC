#ifndef CONVARPROXY_H
#define CONVARPROXY_H

#include "ns_plugin.h"
#include "proxy.h"
#include "types.h"

class ConVarProxy: public ClassProxy<ConVar> {
private:
    const char* pszName;
    const char* pszDefaultValue;
    int nFlags;
    const char* pszHelpString;
    bool bMin;
    float fMin;
    bool bMax;
    float fMax;
    FnChangeCallback_t pCallback;

public:
    ConVarProxy(
        const char* pszName,
        const char* pszDefaultValue,
        int nFlags,
        const char* pszHelpString,
        bool bMin,
        float fMin,
        bool bMax,
        float fMax,
        FnChangeCallback_t pCallback
    ) :
        pszName(pszName),
        pszDefaultValue(pszDefaultValue),
        nFlags(nFlags),
        pszHelpString(pszHelpString),
        bMin(bMin),
        fMin(fMin),
        bMax(bMax),
        fMax(fMax),
        pCallback(pCallback)
    {}

    virtual void initialize(void* data)
    {
        PLUGIN_DATA_TYPES* plugin_data = static_cast<PLUGIN_DATA_TYPES*>(data);

        PluginInitFuncs* funcs = plugin_data->funcs;
        EngineData* engine_data = plugin_data->engine_data;

        assert(funcs->createObject);
        assert(engine_data->ConCommandConstructor);

        this->ptr = static_cast<ConVar*>(funcs->createObject(ObjectType::CONVAR));

        spdlog::info("Registering Convar {}", pszName);

        this->ptr->m_ConCommandBase.m_pConCommandBaseVTable = engine_data->ConVar_Vtable;
        this->ptr->m_ConCommandBase.s_pConCommandBases = (ConCommandBase*)engine_data->IConVar_Vtable;

        engine_data->conVarMalloc(&(this->ptr->m_pMalloc), 0, 0);
        engine_data->conVarRegister(this->ptr, pszName, pszDefaultValue, nFlags, pszHelpString, bMin, fMin, bMax, fMax, (void*)pCallback);
    }

    const char* GetString() const {
        if (!this->ptr)
            return this->pszDefaultValue;

        return this->ptr->m_Value.m_pszString;
    }

    bool Getbool() const {
        return !!GetInt();
    }

    int GetInt() const {
        if (!this->ptr)
            return atoi(this->pszDefaultValue);

        return this->ptr->m_Value.m_nValue;
    }

    float GetFloat() const {
        if (!this->ptr)
            return atof(this->pszDefaultValue);

        return this->ptr->m_Value.m_fValue;
    }
};

#endif