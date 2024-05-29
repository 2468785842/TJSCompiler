//---------------------------------------------------------------------------
/*
	TJS2 Script Engine
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// Date class implementation
//---------------------------------------------------------------------------

#ifndef tjsDateH
#define tjsDateH

#include <ctime>
#include "tjsNative.h"

namespace TJSDate {
    //---------------------------------------------------------------------------
    class tTJSNI_Date : public tTJSNativeInstance {
    public:
        tTJSNI_Date();
        time_t DateTime;
    };

    //---------------------------------------------------------------------------
    class tTJSNC_Date : public tTJSNativeClass {
    public:
        tTJSNC_Date();

        static tjs_uint32 ClassID;

    private:
        tTJSNativeInstance* CreateNativeInstance();
    };

    //---------------------------------------------------------------------------
} // namespace TJS

#endif
