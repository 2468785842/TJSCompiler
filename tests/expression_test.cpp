/*
 * Copyright (c) 2024/12/18
 *
 * 表达式单元测试 - 旧解释器兼容性测试
 */
#include <catch2/catch_all.hpp>
#include "test_config.h"
#include "tjsScriptBlock.h"

TEST_CASE("表达式 - 字面量表达式")
{
    SECTION("整数字面量") {
        const ttstr syntax = "123;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 123);
        blk->Release();
    }

    SECTION("浮点数字面量") {
        const ttstr syntax = "3.14;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsReal() == Catch::Approx(3.14));
        blk->Release();
    }

    SECTION("字符串字面量") {
        const ttstr syntax = "\"hello world\";";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(ttstr(var.AsString()) == "hello world");
        blk->Release();
    }
}

TEST_CASE("表达式 - 标识符表达式")
{
    SECTION("变量引用") {
        // 先定义变量，然后引用
        const ttstr syntax =
            "var variableName = 42;\n"
            "variableName;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 42);
        blk->Release();
    }
}

TEST_CASE("表达式 - 二元运算表达式")
{
    SECTION("算术运算 - 加法") {
        const ttstr syntax =
            "var a = 10;\n"
            "var b = 20;\n"
            "a + b;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 30);
        blk->Release();
    }

    SECTION("算术运算 - 乘法") {
        const ttstr syntax =
            "var a = 10;\n"
            "var b = 20;\n"
            "a * b;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 200);
        blk->Release();
    }

    SECTION("比较运算") {
        const ttstr syntax =
            "var x = 10;\n"
            "var y = 5;\n"
            "x > y;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsBool() == true);
        blk->Release();
    }

    SECTION("逻辑运算 - 与") {
        const ttstr syntax =
            "var a = true;\n"
            "var b = true;\n"
            "a && b;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsBool() == true);
        blk->Release();
    }

    SECTION("逻辑运算 - 或") {
        const ttstr syntax =
            "var a = false;\n"
            "var b = true;\n"
            "a || b;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsBool() == true);
        blk->Release();
    }
}

TEST_CASE("表达式 - 一元运算表达式")
{
    SECTION("逻辑非") {
        const ttstr syntax =
            "var flag = false;\n"
            "!flag;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsBool() == true);
        blk->Release();
    }

    SECTION("负号") {
        const ttstr syntax =
            "var value = 42;\n"
            "-value;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == -42);
        blk->Release();
    }
}

TEST_CASE("表达式 - 赋值表达式")
{
    SECTION("简单赋值") {
        const ttstr syntax =
            "var x;\n"
            "x = 42;\n"
            "x;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 42);
        blk->Release();
    }

    SECTION("复合赋值") {
        const ttstr syntax =
            "var x;\n"
            "var y;\n"
            "x = y = 10;\n"
            "x;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 10);
        blk->Release();
    }

    SECTION("复合赋值 - 验证y的值") {
        const ttstr syntax =
            "var x;\n"
            "var y;\n"
            "x = y = 10;\n"
            "y;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 10);
        blk->Release();
    }

    SECTION("复合算术赋值") {
        const ttstr syntax =
            "var x = 5;\n"
            "var y = 3;\n"
            "x += y;\n"
            "x;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 8);
        blk->Release();
    }
}

TEST_CASE("表达式 - 函数调用表达式")
{
    SECTION("无参数调用") {
        const ttstr syntax =
            "function func() { return 42; }\n"
            "func();";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 42);
        blk->Release();
    }

    SECTION("带参数调用") {
        const ttstr syntax =
            "function add(a, b, c) { return a + b + c; }\n"
            "add(1, 2, 3);";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 6);
        blk->Release();
    }

    SECTION("嵌套调用") {
        const ttstr syntax =
            "function min(a, b) { return a < b ? a : b; }\n"
            "function max(a, b) { return a > b ? a : b; }\n"
            "var a = 5, b = 10, c = 7;\n"
            "max(min(a, b), c);";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 7);
        blk->Release();
    }

    SECTION("匿名函数调用") {
        const ttstr syntax =
            "(function(x, y) { return x * y; })(6, 7);";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 42);
        blk->Release();
    }

    SECTION("对象方法调用") {
        const ttstr syntax =
            "var obj = {\n"
            "  value: 10,\n"
            "  add: function(x) { return this.value + x; }\n"
            "};\n"
            "obj.add(32);";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 42);
        blk->Release();
    }
}

TEST_CASE("表达式 - 复杂表达式组合")
{
    SECTION("混合算术表达式") {
        const ttstr syntax =
            "var a = 10, b = 20, c = 30, d = 40, e = 5;\n"
            "a + b * c - d / e;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 10 + 20*30 - 40/5);
        blk->Release();
    }

    SECTION("带括号的表达式") {
        const ttstr syntax =
            "var a = 10, b = 20, c = 30, d = 5;\n"
            "(a + b) * (c - d);";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == (10 + 20) * (30 - 5));
        blk->Release();
    }

    SECTION("逻辑表达式组合") {
        const ttstr syntax =
            "var a = 5, b = 8, c = 5;\n"
            "a > 0 && b < 10 || c == 5;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsBool() == true);
        blk->Release();
    }

    SECTION("三元条件表达式") {
        const ttstr syntax =
            "var condition = true;\n"
            "var a = 10, b = 20;\n"
            "condition ? a : b;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 10);
        blk->Release();
    }

    SECTION("复杂嵌套表达式") {
        const ttstr syntax =
            "var a = 2, b = 3, c = 4, d = 5;\n"
            "(a * (b + c)) / (d - a) + (b % a);";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == (2 * (3 + 4)) / (5 - 2) + (3 % 2));
        blk->Release();
    }
}

TEST_CASE("表达式 - 类型转换和混合类型")
{
    SECTION("整数转字符串") {
        const ttstr syntax =
            "var num = 123;\n"
            "\"Value: \" + num;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(ttstr(var.GetString()) == "Value: 123");
        blk->Release();
    }

    SECTION("布尔值转整数") {
        const ttstr syntax =
            "var flag = true;\n"
            "flag + 10;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        // TJS中true转换为整数通常是1
        REQUIRE(var.AsInteger() == 11);
        blk->Release();
    }

    SECTION("字符串转数字") {
        const ttstr syntax =
            "\"123\" * 2;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 246);
        blk->Release();
    }
}

TEST_CASE("表达式 - 数组和对象访问")
{
    SECTION("数组访问") {
        const ttstr syntax =
            "var arr = [10, 20, 30, 40, 50];\n"
            "arr[2];";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 30);
        blk->Release();
    }

    SECTION("对象属性访问") {
        const ttstr syntax =
            "var obj = {\n"
            "  name: \"test\",\n"
            "  value: 100,\n"
            "  nested: { x: 5, y: 10 }\n"
            "};\n"
            "obj.value;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 100);
        blk->Release();
    }

    SECTION("嵌套对象访问") {
        const ttstr syntax =
            "var obj = {\n"
            "  data: {\n"
            "    coordinates: { x: 3, y: 7 },\n"
            "    value: 42\n"
            "  }\n"
            "};\n"
            "obj.data.coordinates.x + obj.data.value;";
        auto *blk = new tTJSScriptBlock(tvPScriptEngine);

        tTJSVariant var;
        blk->SetText(&var, syntax.c_str(), nullptr, true);
        REQUIRE(var.AsInteger() == 3 + 42);
        blk->Release();
    }
}


TEST_CASE("t")
{
    const ttstr syntax =
        "function func() { return b; }"
        "var b = 1;"
        "return func();";

    tTJSVariant var;
    tvPScriptEngine->ExecScript(syntax.c_str(), &var);
    REQUIRE(var.AsInteger() == 1);
}