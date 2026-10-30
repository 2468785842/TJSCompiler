#include <algorithm>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include "tjs.h"
#include "tjsError.h"
#include "DebugInterface.h"
#include "ScriptsInterface.h"

using namespace TJS;

class Console final : public iTJSConsoleOutput {
public:
    void ExceptionPrint(const tjs_char *msg) override { std::wcerr << msg << '\n'; }
    void Print(const tjs_char *msg) override { std::wcout << msg << '\n'; }
};

class OutputStream final : public tTJSBinaryStream {
public:
    std::vector<char> bytes;
    size_t position{};
    tjs_uint64 TJS_INTF_METHOD Seek(tjs_int64 offset, tjs_int whence) override {
        const auto base = whence == TJS_BS_SEEK_SET ? 0 :
            whence == TJS_BS_SEEK_CUR ? position : bytes.size();
        if(offset < 0 && static_cast<tjs_uint64>(-offset) > base)
            throw std::runtime_error("invalid stream seek");
        position = base + offset;
        return position;
    }
    tjs_uint TJS_INTF_METHOD Read(void *buffer, tjs_uint size) override {
        const auto count = position < bytes.size() ? std::min<size_t>(size, bytes.size() - position) : 0;
        if(count) std::memcpy(buffer, bytes.data() + position, count);
        position += count;
        return static_cast<tjs_uint>(count);
    }
    tjs_uint TJS_INTF_METHOD Write(const void *buffer, tjs_uint size) override {
        if(position + size > bytes.size()) bytes.resize(position + size);
        if(size) std::memcpy(bytes.data() + position, buffer, size);
        position += size;
        return size;
    }
    tjs_uint64 TJS_INTF_METHOD GetSize() override { return bytes.size(); }
};

int main(int argc, char **argv) {
    const auto utf8 = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>);
    std::wcout.imbue(utf8);
    std::wcerr.imbue(utf8);
    const bool dump = argc > 1 && std::string(argv[1]) == "--dump";
    if((dump && argc != 4) || (!dump && argc > 2)) {
        std::cerr << "Usage: TJSCompiler [script.tjs] | --dump script.tjs out.tjsbc\n";
        return 2;
    }
    // 默认脚本相对项目源码目录，显式传入的相对路径按调用者工作目录解析。
    const auto script = argc == 1 ? std::filesystem::path(TJS_SOURCE_DIR) / "tests/test_files/startup.tjs"
                                  : std::filesystem::path(argv[dump ? 2 : 1]);
    std::wifstream input(script, std::ios::binary);
    input.imbue(utf8);
    if(!input) { std::cerr << "Cannot open script: " << script << '\n'; return 1; }
    std::wstringstream text;
    text << input.rdbuf();
    TVPSetScriptsBaseDir(script.string());
    Console console;
    tTJS *engine = nullptr;
    try {
        engine = new tTJS();
        TVPSetScriptsEngine(engine);
        engine->SetPPValue(TJS_W("kirikiriz"), 1);
        engine->SetConsoleOutput(&console);
        auto *debug = TVPCreateNativeClass_Debug();
        tTJSVariant value(debug);
        debug->Release();
        auto *global = engine->GetGlobalNoAddRef();
        global->PropSet(TJS_MEMBERENSURE | TJS_IGNOREPROP, TJS_W("Debug"), nullptr, &value, global);
        auto *scripts = TVPCreateNativeClass_Scripts();
        tTJSVariant scriptsValue(scripts);
        scripts->Release();
        global->PropSet(TJS_MEMBERENSURE | TJS_IGNOREPROP, TJS_W("Scripts"), nullptr, &scriptsValue, global);
        if(dump) {
            OutputStream output;
            engine->CompileScript(text.str().c_str(), &output);
            std::ofstream file(argv[3], std::ios::binary);
            file.write(output.bytes.data(), static_cast<std::streamsize>(output.bytes.size()));
            if(!file) throw std::runtime_error("cannot write bytecode output");
        } else {
            engine->ExecScript(text.str().c_str());
        }
        TVPSetScriptsEngine(nullptr);
        engine->Shutdown();
        engine->Release();
        return 0;
    } catch(const eTJS &error) {
        std::wcerr << error.GetMessage().c_str() << '\n';
    } catch(const std::exception &error) {
        std::cerr << error.what() << '\n';
    }
    TVPSetScriptsEngine(nullptr);
    if(engine) engine->Release();
    return 1;
}
