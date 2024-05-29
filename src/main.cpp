#include <codecvt>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>

#include "tjs.h"
#include "DebugInterface.h"
#include "tjsScriptBlock.h"
using namespace std;

class TJSConsoleOutputDef final : virtual public iTJSConsoleOutput {
public:
    void ExceptionPrint(const tjs_char *msg) override { std::wcerr << msg << std::endl; }

    void Print(const tjs_char *msg) override { std::wcout << msg << endl; }
} static iTJSConsoleOutputDef{};

static const auto utf_8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>);

int main() {
    std::wcout.imbue(utf_8_locale);
    std::wstringstream tvPInitTJSScript;
    std::wfstream startupTJS{
            // R"(D:\Users\Li_Dong\CLionProjects\TJSCompiler\bin\Windows\test_function.tjs)",
            R"(D:\Users\Li_Dong\CLionProjects\TJSCompiler\bin\Windows\startup.tjs)",
            std::ios::in
    };
    startupTJS.imbue(utf_8_locale);
    if (!startupTJS.is_open()) {
        std::cout << "open file error\t" << startupTJS.rdstate();
    }
    tvPInitTJSScript << startupTJS.rdbuf();
    startupTJS.close();
    const auto tvPScriptEngine = new tTJS();

    tTJSVariant val;
    // register some TVP classes/objects/functions/propeties
    iTJSDispatch2 *dsp;
    iTJSDispatch2 *global = tvPScriptEngine->GetGlobalNoAddRef();

    // add kirikiriz
    tvPScriptEngine->SetPPValue(TJS_W("kirikiriz"), 1);
    // set console output gateway handler
    tvPScriptEngine->SetConsoleOutput(&iTJSConsoleOutputDef);

#define REGISTER_OBJECT(classname, instance)                                   \
    dsp = (instance);                                                          \
    val = tTJSVariant(dsp /*, dsp*/);                                          \
    dsp->Release();                                                            \
    global->PropSet(TJS_MEMBERENSURE | TJS_IGNOREPROP, TJS_W(#classname),      \
                    NULL, &val, global);
    REGISTER_OBJECT(Debug, TVPCreateNativeClass_Debug())

    auto *blk = new tTJSScriptBlock(tvPScriptEngine);

    blk->SetText(nullptr, ttstr(tvPInitTJSScript.str()).c_str(), nullptr, false);
    tvPScriptEngine->AddScriptBlock(blk);
    tvPScriptEngine->Dump();
    tvPScriptEngine->RemoveScriptBlock(blk);
    blk->Release();
    return 0;
}
