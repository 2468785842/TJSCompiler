//
// Created by LiDong on 2025/12/6.
//

#include <catch2/catch_all.hpp>
#include "test_config.h"
#include "tjsScriptBlock.h"

TEST_CASE("类 - 属性外部初始化访问") {
    const ttstr syntax = R"(
        var a = 1;
        class A {
            var b = a;
        }
        var c = (new A()).b;
        a = 2;
        var d = (new A()).b;
        return c == d;
    )";
    tTJSVariant var;
    const auto tjs = new tTJS{};
    tjs->ExecScript(syntax, &var);

    REQUIRE(var.AsBool() == true);
    tjs->Release();
}

TEST_CASE("函数 - 属性外部初始化访问") {
    const ttstr syntax = R"(
        var a = 1;
        class A {
            function getA() {
                var a = 2;
                function inGetA(b = a) {
                    return b;
                }
                return inGetA();
            }
            return getA();
        }
    )";
    tTJSVariant var;
    const auto tjs = new tTJS{};
    tjs->ExecScript(syntax, &var);

    REQUIRE(var.AsInteger() == 2);
    tjs->Release();
}

TEST_CASE("类型") {
    const ttstr syntax = R"(
        var a = 2;
        return (a >>> 1);
    )";
    tTJSVariant var;
    const auto tjs = new tTJS{};
    tjs->ExecScript(syntax, &var);

    REQUIRE(var.AsInteger() == 2);
    tjs->Release();
}

TEST_CASE("TJS2 Legacy - Class/Slot/Constructor/Lookup 行为全集") {
    const ttstr syntax = R"(
        // --- 构造函数参数传递 ---
        class A1 {
            var x;
            function A1(v) { x = v; }
        }
        var o1 = new A1(42);
        var r1 = o1.x;

        // --- 成员初始化先于构造 ---
        class A2 {
            var x = 10;
            function A2() { x += 5; }
        }
        var o2 = new A2();
        var r2 = o2.x;

        // --- getter 行为 ---
        class A3 {
            var x = 3;
            property p {
                getter() { return x * 2; }
            }
        }
        var o3 = new A3();
        var r3 = o3.p;

        // --- instanceof ---
        class A4 {}
        var o4 = new A4();
        var r4 = (A4 instanceof "Class") && (o4 instanceof "A4");

        // --- 方法闭包绑定 this ---
        class A5 {
            var x = 1;
            function inc() { x++; }
        }
        var o5 = new A5();
        var f5 = o5.inc;
        f5();
        var r5 = o5.x;

        // --- incontextof ---
        class A6 {
            var x = 0;
            function inc() { x++; }
        }
        var a6_1 = new A6();
        var a6_2 = new A6();
        var f6 = a6_1.inc;
        (f6 incontextof a6_2)();
        var r6_1 = a6_1.x;
        var r6_2 = a6_2.x;

        // --- override ---
        class A7 { function foo() { return 1; } }
        class B7 extends A7 { function foo() { return 2; } }
        var o7 = new B7();
        var r7 = o7.foo();

        // --- slot 查找优先定义类 ---
        class A8 {
            var x = 1;
            function getX() { return x; }
        }
        class B8 extends A8 {
            var x = 2;
            function B8() { A8(); }
        }
        var o8 = new B8();
        var r8 = o8.getX(); // == 1

        // --- fallback 到 instance ---
        class A9 {
            function getX() { return x; }
        }
        class B9 extends A9 {
            var x = 2;
            function B9() { A9(); }
        }
        var o9 = new B9();
        var r9 = o9.getX(); // == 2

        // --- super slot 优先于 instance ---
        class C10 { var x = 4; function C10() {} }
        class A10 extends C10 {
            function A10() { C10(); }
            function getX() { return x; }
        }
        class B10 extends A10 {
            var x = 2;
            function B10() { A10(); }
        }
        var o10 = new B10();
        var r10 = o10.getX(); // == 2

        // --- 构造函数不会覆盖子类字段 ---
        class A11 {
            var x = 1;
            function A11() { x = 10; }
        }
        class B11 extends A11 {
            var x = 2;
            function B11() { A11(); }
        }
        var o11 = new B11();
        var r11 = o11.x;

    )";

    tTJSVariant var;
    auto *tjs = new tTJS{};
    tjs->ExecScript(syntax, &var);

    auto *global = tjs->GetGlobal();

    global->PropGet(0, TJS_W("r1"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 42);

    global->PropGet(0, TJS_W("r2"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 15);

    global->PropGet(0, TJS_W("r3"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 6);

    global->PropGet(0, TJS_W("r4"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 1);

    global->PropGet(0, TJS_W("r5"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 2);

    global->PropGet(0, TJS_W("r6_1"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 0);

    global->PropGet(0, TJS_W("r6_2"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 1);

    global->PropGet(0, TJS_W("r7"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 2);

    global->PropGet(0, TJS_W("r8"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 1);

    global->PropGet(0, TJS_W("r9"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 2);

    global->PropGet(0, TJS_W("r10"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 2);

    global->PropGet(0, TJS_W("r11"), nullptr, &var, nullptr);
    REQUIRE(var.AsInteger() == 10);

    tjs->Release();
}
