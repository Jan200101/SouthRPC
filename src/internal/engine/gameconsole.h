
#include "internal/engine/gameconsoledialog.h"

typedef struct CGameConsole {
    void* vtable;

    bool m_bInitialized;
    CGameConsoleDialog* m_pConsole;
} CGameConsole;
