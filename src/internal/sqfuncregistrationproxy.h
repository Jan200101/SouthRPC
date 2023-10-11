#ifndef SQFUCREGISTRATIONPROXY_H
#define SQFUCREGISTRATIONPROXY_H

#include "ns_plugin.h"
#include "proxy.h"
#include "types.h"

static eSQReturnType SQReturnTypeFromString(const char* pReturnType)
{
    static const std::map<std::string, eSQReturnType> sqReturnTypeNameToString = {
        {"bool", eSQReturnType::Boolean},
        {"float", eSQReturnType::Float},
        {"vector", eSQReturnType::Vector},
        {"int", eSQReturnType::Integer},
        {"entity", eSQReturnType::Entity},
        {"string", eSQReturnType::String},
        {"array", eSQReturnType::Arrays},
        {"asset", eSQReturnType::Asset},
        {"table", eSQReturnType::Table}};

    if (sqReturnTypeNameToString.find(pReturnType) != sqReturnTypeNameToString.end())
        return sqReturnTypeNameToString.at(pReturnType);
    else
        return eSQReturnType::Default; // previous default value
}

class SQFuncRegistrationProxy : public ClassProxy<SQFuncRegistration> {
    private:
        std::string returnType;
        std::string name;
        std::string argTypes;
        std::string helpText;
        ScriptContext context;
        SQFunction func;

    public:
        SQFuncRegistrationProxy(
            std::string returnType,
            std::string name,
            std::string argTypes,
            std::string helpText,
            ScriptContext context,
            SQFunction func
        ) :
            returnType(returnType),
            name(name),
            argTypes(argTypes),
            helpText(helpText),
            context(context),
            func(func)
        {}

        virtual void initialize(void* data)
        {
            ptr = new SQFuncRegistration;

            ptr->squirrelFuncName = new char[name.size() + 1];
            strcpy((char*)ptr->squirrelFuncName, name.c_str());
            ptr->cppFuncName = ptr->squirrelFuncName;

            ptr->helpText = new char[helpText.size() + 1];
            strcpy((char*)ptr->helpText, helpText.c_str());

            ptr->returnTypeString = new char[returnType.size() + 1];
            strcpy((char*)ptr->returnTypeString, returnType.c_str());
            ptr->returnType = SQReturnTypeFromString(returnType.c_str());

            ptr->argTypes = new char[argTypes.size() + 1];
            strcpy((char*)ptr->argTypes, argTypes.c_str());

            ptr->funcPtr = func;
        }

        ScriptContext getContext() { return this->context; }
        std::string getName() { return this->name;  }
};

#endif