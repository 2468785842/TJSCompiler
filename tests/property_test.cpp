//
// Created by LiDong on 2025/12/6.
//

#include <catch2/catch_all.hpp>
#include "test_config.h"
#include "tjsScriptBlock.h"

TEST_CASE("属性定义") {
    const ttstr syntax = R"(
    property p_readonly
    {
        var value = 42;
        getter
        {
            return value;
        }
    }

    var x = p_readonly;   // OK
    return *(getter incontextof &p_readonly);
    )";
    tTJSVariant var;
    const auto tjs = new tTJS{};
    tjs->ExecScript(syntax, &var);
    REQUIRE(var.AsBool() == true);
    tjs->Release();
}