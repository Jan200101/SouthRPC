#ifndef TYPES_H
#define TYPES_H

#include "ns_plugin.h"

typedef struct {
    PluginInitFuncs* funcs;
    PluginNorthstarData* data;
    EngineData* engine_data;
} PLUGIN_DATA_TYPES ;

#ifndef SQTrue
#define SQTrue (1)
#endif

#ifndef SQFalse
#define SQFalse	(0)
#endif

#ifndef SQ_FAILED
#define SQ_FAILED(res) (res<0)
#endif

#ifndef SQ_SUCCEEDED
#define SQ_SUCCEEDED(res) (res>=0)
#endif

#ifndef ISREFCOUNTED
#define ISREFCOUNTED(t) (t&SQOBJECT_REF_COUNTED)
#endif

#ifndef sq_isnull
#define sq_isnull(o) ((o)._type==OT_NULL)
#endif

#define SQ_NULL_OBJ SQObject { OT_NULL, 0, nullptr }

#endif
