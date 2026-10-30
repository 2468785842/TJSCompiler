#pragma once

#include <string>

#include "tjsNative.h"

namespace TJS {
    // Scripts 宿主类（krkr2 tTJSNC_Scripts / ScriptMgnIntf.cpp:1295-1570 对应物）：
    // execStorage 执行存储脚本（语句形态，全局上下文）、evalStorage 表达式求值。
    // compileStorage 依赖字节码导出管线，宿主未接（krkr2 TVPCompileStorage 语义），
    // 与 Ciallang 侧同样不实现。
    class tTJSNC_Scripts final : public tTJSNativeClass {
    public:
        tjs_uint32 ClassID = -1;
        tTJSNC_Scripts();
    };

    // 宿主注入：执行引擎与当前脚本目录（Scripts 相对路径解析基准）。
    // main.cpp 在创建 tTJS、取得脚本路径后调用。
    void TVPSetScriptsEngine(tTJS *engine);

    void TVPSetScriptsBaseDir(const std::string &scriptPathUtf8);

    static tTJSNativeClass *TVPCreateNativeClass_Scripts() {
        return new tTJSNC_Scripts();
    }
}
