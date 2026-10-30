#include "ScriptsInterface.h"

#include <codecvt>
#include <fstream>
#include <locale>
#include <sstream>

namespace TJS {
    namespace {
        // krkr2 TVPScriptEngine / TVPSearchPlacedPath 的宿主注入版：
        // 执行引擎与当前脚本目录由 main.cpp 注入（CLI 场景相对路径相对
        // 当前执行脚本所在目录；绝对路径直用）
        tTJS *TVPScriptEngine = nullptr;
        std::wstring TVPBaseDir;

        std::wstring TVPResolveStoragePath(const ttstr &name) {
            const std::wstring path = name.AsStdString();
            if(path.size() >= 2 && path[1] == L':')
                return path; // 盘符绝对路径
            if(!path.empty() && (path[0] == L'/' || path[0] == L'\\'))
                return path; // 根路径
            if(TVPBaseDir.empty())
                return path; // 未注入基目录：相对进程 cwd
            if(TVPBaseDir.back() == L'/' || TVPBaseDir.back() == L'\\')
                return TVPBaseDir + path;
            return TVPBaseDir + L"/" + path;
        }

        // krkr2 TVPCreateTextStreamForRead 的 CLI 简化版：UTF-8 文本读取
        //（与 main.cpp 读启动脚本同款 locale 管线）
        std::wstring TVPReadStorageText(const std::wstring &path) {
            std::wfstream stream(path, std::ios::in);
            if(!stream) {
                TJS_eTJSError((std::wstring(L"cannot open storage: ") + path).c_str());
            }
            static const auto utf8Locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>);
            stream.imbue(utf8Locale);
            std::wstringstream buffer;
            buffer << stream.rdbuf();
            return buffer.str();
        }

        // krkr2 TVPExecuteStorage（ScriptMgnIntf.cpp:711）对应物：
        // 读文件 → 按 isExpression 编译执行，result 透传，context 省略 =
        // 全局上下文（ExecScript/EvalExpression 的 context=nullptr 语义）
        void TVPExecuteStorage(const ttstr &name, tTJSVariant *result, const bool isexpression) {
            if(!TVPScriptEngine) {
                TJS_eTJSError(TJS_W("script engine is not initialized"));
            }
            const std::wstring path = TVPResolveStoragePath(name);
            const std::wstring content = TVPReadStorageText(path);
            // 脚本名取 basename（krkr2 TVPExtractStorageName 对齐，诊断显示用）
            const auto pos = path.find_last_of(L"/\\");
            const std::wstring shortname = pos == std::wstring::npos ? path : path.substr(pos + 1);
            const ttstr nameArg{shortname};
            if(isexpression)
                TVPScriptEngine->EvalExpression(ttstr{content}, result, nullptr, &nameArg);
            else
                TVPScriptEngine->ExecScript(ttstr{content}, result, nullptr, &nameArg);
        }
    } // namespace

    void TVPSetScriptsEngine(tTJS *engine) {
        TVPScriptEngine = engine;
    }

    void TVPSetScriptsBaseDir(const std::string &scriptPathUtf8) {
        // UTF-8 → UTF-16（与启动脚本读取同款 codecvt 管线）
        if(scriptPathUtf8.empty())
            return;
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::wstring path = converter.from_bytes(scriptPathUtf8);
        const auto pos = path.find_last_of(L"/\\");
        TVPBaseDir = pos == std::wstring::npos ? std::wstring{} : path.substr(0, pos + 1);
    }

   //---------------------------------------------------------------------------
    // tTJSNC_Scripts
    //---------------------------------------------------------------------------

    tTJSNC_Scripts::tTJSNC_Scripts() : tTJSNativeClass(TJS_W("Scripts")) {
        // registration of native members

        TJSNativeClassRegisterNCM(
            this, TJS_W("finalize"),
            TJSCreateNativeClassMethod([] {
                return TJS_S_OK;
            }),
            ClassName, nitMethod);

        TJSNativeClassRegisterNCM(
            this, TJS_W("Scripts"),
            TJSCreateNativeClassConstructor([] {
                return TJS_S_OK;
            }),
            ClassName, nitMethod);

        //-- methods（krkr2 TJS_END_NATIVE_STATIC_METHOD_DECL 对应：静态成员）

        TJSNativeClassRegisterNCM(
            this, TJS_W("execStorage"),
            TJSCreateNativeClassMethod([](
                tTJSVariant *result, const tjs_int numparams, tTJSVariant **param, iTJSDispatch2*
            ) {
                if (numparams < 1) return TJS_E_BADPARAMCOUNT;
                // execute script which stored in storage
                TVPExecuteStorage(*param[0], result, false);
                return TJS_S_OK;
            }),
            ClassName, nitMethod, TJS_STATICMEMBER);

        TJSNativeClassRegisterNCM(
            this, TJS_W("evalStorage"),
            TJSCreateNativeClassMethod([](
                tTJSVariant *result, const tjs_int numparams, tTJSVariant **param, iTJSDispatch2*
            ) {
                if (numparams < 1) return TJS_E_BADPARAMCOUNT;
                // execute expression which stored in storage
                TVPExecuteStorage(*param[0], result, true);
                return TJS_S_OK;
            }),
            ClassName, nitMethod, TJS_STATICMEMBER);
    }
} // namespace TJS
