// derived from https://github.com/ValveSoftware/source-sdk-2013/blob/0d8dceea4310fde5706b3ce1c70609d72a38efdf/mp/src/public/icvar.h

typedef struct IConsoleDisplayFunc {
    struct {
        //virtual void ColorPrint( const Color& clr, const char *pMessage ) = 0;
        void* ColorPrint;

        //virtual void Print( const char *pMessage ) = 0;
        void (*Print)(struct IConsoleDisplayFunc* instance, const char* pMessage);

        //virtual void DPrint( const char *pMessage ) = 0;
        void (*DPrint)(struct IConsoleDisplayFunc* instance, const char* pMessage);
    }* vtable;

} IConsoleDisplayFunc;
