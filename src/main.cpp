#include <chrono>
#include <cstring>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>

#include "tjs.h"
#include "DebugInterface.h"
#include "ScriptsInterface.h"
#include "tjsScriptBlock.h"
using namespace std;

class TJSConsoleOutputDef final : virtual public iTJSConsoleOutput {
public:
    void ExceptionPrint(const tjs_char *msg) override { std::wcerr << msg << std::endl; }

    void Print(const tjs_char *msg) override { std::wcout << msg << endl; }
} static iTJSConsoleOutputDef{};

static const auto utf_8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>);

int main(int argc, char *argv[]) {

    if(argc < 2) {
        return 0;
    }

    // --dump <script.tjs> <out.tjsbc>：编译脚本并导出 "TJS2 100\0" 字节码文件。
    // Ciallang 字节码转译的对拍样本生成器（走公开 API tTJS::CompileScript，
    // 内部经 tTJSScriptBlock::ExportByteCode 落盘）
    const bool dumpMode = argc >= 4 && std::string(argv[1]) == "--dump";
    const char *scriptPath = dumpMode ? argv[2] : argv[1];
    // Scripts.execStorage 相对路径基准：当前执行脚本所在目录
    TJS::TVPSetScriptsBaseDir(scriptPath);

    std::wcout.imbue(utf_8_locale);
    std::wstringstream tvPInitTJSScript;
    // MSVC STL 的 wfstream 不收 char* 路径：先转宽字符
    const std::wstring scriptPathW(scriptPath, scriptPath + std::strlen(scriptPath));
    std::wfstream startupTJS{
            scriptPathW.c_str(),
            std::ios::in
    };
    startupTJS.imbue(utf_8_locale);
    if (!startupTJS.is_open()) {
        std::cout << "open file error\t" << startupTJS.rdstate();
    }
    tvPInitTJSScript << startupTJS.rdbuf();
    startupTJS.close();
    const auto tvPScriptEngine = new tTJS();
    TJS::TVPSetScriptsEngine(tvPScriptEngine);

    if (dumpMode) {
        class LocalBinaryStream final : public tTJSBinaryStream {
            std::ofstream ofs_;
        public:
            explicit LocalBinaryStream(const char *path)
                : ofs_(path, std::ios::binary | std::ios::trunc) {}
            tjs_uint64 TJS_INTF_METHOD Seek(tjs_int64, tjs_int) override { return 0; }
            tjs_uint TJS_INTF_METHOD Read(void *, tjs_uint) override { return 0; }
            tjs_uint TJS_INTF_METHOD Write(const void *buffer, tjs_uint size) override {
                ofs_.write(static_cast<const char *>(buffer), size);
                return ofs_ ? size : 0;
            }
            tjs_uint64 TJS_INTF_METHOD GetSize() override { return 0; }
        } stream{argv[3]};

        tvPScriptEngine->CompileScript(tvPInitTJSScript.str().c_str(), &stream);
        std::cerr << "bytecode written: " << argv[3] << std::endl;
        return 0;
    }

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
    REGISTER_OBJECT(Scripts, TVPCreateNativeClass_Scripts())

    auto *blk = new tTJSScriptBlock(tvPScriptEngine);
    auto start = std::chrono::high_resolution_clock::now();

    blk->SetText(nullptr, ttstr(tvPInitTJSScript.str()).c_str(), nullptr, false);
    auto end = std::chrono::high_resolution_clock::now();

    tvPScriptEngine->AddScriptBlock(blk);
    tvPScriptEngine->Dump();
    tvPScriptEngine->RemoveScriptBlock(blk);
    blk->Release();

    auto duration = duration_cast<std::chrono::microseconds>(end - start);

    std::cerr << "Time taken by function: " << duration.count() << "ms" << std::endl;

    return 0;
}
