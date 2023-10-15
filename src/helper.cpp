#include <rapidjson/document.h>

#include "ns_plugin.h"

#include "helper.h"

void SquirrelToJSON(
    rapidjson::Value* out_val,
    rapidjson::MemoryPoolAllocator<>& allocator,
    SQObject* obj_ptr
) {

    if (obj_ptr)
    {
        switch (obj_ptr->_Type)
        {
            case OT_BOOL:
                out_val->SetBool(obj_ptr->_VAL.asInteger);
                break;

            case OT_INTEGER:
                out_val->SetInt(obj_ptr->_VAL.asInteger);
                break;

            case OT_STRING:
                out_val->SetString(obj_ptr->_VAL.asString->_val, obj_ptr->_VAL.asString->length);
                break;

            case OT_ARRAY:
                out_val->SetArray();

                for (int i = 0; i < obj_ptr->_VAL.asArray->_usedSlots; ++i)
                {
                    rapidjson::Value n;
                    SquirrelToJSON(&n, allocator, &obj_ptr->_VAL.asArray->_values[i]);
                    out_val->PushBack(n, allocator);
                }
                break;

            case OT_TABLE:
                out_val->SetObject();

                for (int i = 0; i < obj_ptr->_VAL.asTable->_numOfNodes; ++i)
                {
                    auto node = &obj_ptr->_VAL.asTable->_nodes[i];
                    if (node->key._Type != OT_STRING)
                        continue;

                    rapidjson::Value k;
                    SquirrelToJSON(&k, allocator, &node->key);

                    rapidjson::Value v;
                    SquirrelToJSON(&v, allocator, &node->val);
                    out_val->AddMember(k, v, allocator);
                }
                break;

            default:
                break;
        }
    }
}
