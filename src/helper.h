#ifndef HELPER_H
#define HELPER_H

#include <rapidjson/document.h>

#include "ns_plugin.h"

void SquirrelToJSON(
    rapidjson::Value* out_val,
    rapidjson::MemoryPoolAllocator<>& allocator,
    SQObject* obj_ptr
);

#endif