/*
 * Copyright (c) 2024/12/18
 *
 * 函数声明单元测试
 */

#include <catch2/catch_all.hpp>
#include "test_config.h"
#include "tjsScriptBlock.h"

TEST_CASE("解释器 - 脚本执行性能") {
    BENCHMARK("fib 15") {
        const ttstr syntax = R"(
            function fib(n) {
                if(n < 2) return n;
                return fib(n - 2) + fib(n - 1);
            }
            fib(15);
        )";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, false);
        blk->Release();
    };
}