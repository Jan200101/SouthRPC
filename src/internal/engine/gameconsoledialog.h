
#include "internal/engine/consoledialog.h"

typedef struct CGameConsoleDialog {
    void* vtable;
    CConsoleDialog _derived;
} CGameConsoleDialog;
