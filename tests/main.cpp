//
// Created by lidong on 25-6-21.
//

#include <catch2/catch_session.hpp>
#include <codecvt>
#include <fstream>
#include <iostream>

#include "tjs.h"
#include "DebugInterface.h"
#include "tjsScriptBlock.h"
using namespace std;

class TJSConsoleOutputDef final : virtual public iTJSConsoleOutput {
public:
    void ExceptionPrint(const tjs_char *msg) override { std::wcerr << msg << std::endl; }

    void Print(const tjs_char *msg) override { std::wcout << msg << endl; }
} static iTJSConsoleOutputDef{};

tTJS *tvPScriptEngine = new tTJS();

int main(int argc, char *argv[]) {

    iTJSDispatch2 *global = tvPScriptEngine->GetGlobalNoAddRef();

    // add kirikiriz
    tvPScriptEngine->SetPPValue(TJS_W("kirikiriz"), 1);
    // set console output gateway handler
    tvPScriptEngine->SetConsoleOutput(&iTJSConsoleOutputDef);

    tTJSVariant val;
    // register some TVP classes/objects/functions/propeties
    iTJSDispatch2 *dsp;
#define REGISTER_OBJECT(classname, instance)                                   \
dsp = (instance);                                                          \
val = tTJSVariant(dsp /*, dsp*/);                                          \
dsp->Release();                                                            \
global->PropSet(TJS_MEMBERENSURE | TJS_IGNOREPROP, TJS_W(#classname),      \
NULL, &val, global);
    REGISTER_OBJECT(Debug, TVPCreateNativeClass_Debug())

    int result = Catch::Session().run(argc, argv);
    tvPScriptEngine->Release();
    return result;
}