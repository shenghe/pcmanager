#ifndef ACF_UNKNWN_INC_
#define ACF_UNKNWN_INC_


//////////////////////////////////////////////////////////////////////////

#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef long HRESULT;
#endif // !_HRESULT_DEFINED

#include "guiddef.h"
#include "winerror.h"

/////////////////////////////////////////////////////////////////////////

extern "C"
{
    struct __declspec(uuid("00000000-0000-0000-C000-000000000046")) __declspec(novtable) FwUnknown
    {
    public:
        virtual long __stdcall QueryInterface(REFIID riid, void  **ppvObject) = 0;
        virtual unsigned long __stdcall AddRef(void) = 0;
        virtual unsigned long __stdcall Release(void) = 0;
    };
}

//////////////////////////////////////////////////////////////////////////

#endif  // !ACF_UNKNWN_INC_


