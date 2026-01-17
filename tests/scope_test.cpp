/*
* Copyright (c) 2024/12/18
 *
 * 函数声明单元测试
 */

#include <catch2/catch_all.hpp>
#include "test_config.h"
#include "tjsScriptBlock.h"

TEST_CASE("作用域 - 上层访问") {
    const ttstr syntax = R"(
        var r = 1;
        var a = 5;
        function outer() {
            var a = 10;
            function inner() {
                r = a;
            }
            inner();
        }
        outer();
        return r;
    )";
    tTJSVariant var;
    const auto tjs = new tTJS{};
    tjs->ExecScript(syntax, &var);

    REQUIRE(var.AsInteger() == 5);
    tjs->Release();
}


TEST_CASE("作用域 - 父类静态变量访问") {
    const ttstr syntax = R"(
        class Base {}
        Base.a = 1;
        class Derived extends Base {
            function getSuperA() { return a; }
        }
        return (new Derived()).getSuperA();
    )";
    tTJSVariant var;
    const auto tjs = new tTJS{};
    tjs->ExecScript(syntax, &var);

    REQUIRE(var.AsInteger() == 1);
    tjs->Release();
}