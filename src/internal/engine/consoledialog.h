
#include "internal/engine/public/icvar.h"

typedef struct CConsolePanel {
    // has no own vtable

    char padding[0x2B4];
    IConsoleDisplayFunc* _derived2;
} CConsolePanel;

typedef struct CConsoleDialog {
    void* vtable;

    char padding[0x394]; // some vgui stuff we don't care about
    CConsolePanel* m_pConsolePanel;
} CConsoleDialog;
