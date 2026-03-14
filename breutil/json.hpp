#pragma once
/**
    @file json.hpp
    @brief A lightweight JSON parsing/generating library header file.

    This library provides a simple JSON parsing solution for small projects
    without the need to introduce thirdparty libraries. It is designed for
    scenarios where JSON parsing requirements are minimal and portability
    is a concern. For more complex JSON parsing needs, consider using
    thirdparty libraries.

    @note By default, strict mode is disabled. In strict mode, comments
    and trailing commas in JSON strings are not allowed.

    Usage

    Parsing JSON Strings
    Parsing a JSON string requires a trycatch block, as parsing errors
    will throw exceptions.

    try {
        bre::json::Value json = bre::json::parse(jsonStr);
    } catch (const bre::JsonParseException& e) {
        std::cout << e.what() << std::endl;
    }

    Accessing JSON Values
    Once parsed, the JSON object can be accessed and manipulated as follows:

    Accessing Values:
    For objects: `json["key"]`
    For arrays: `json[0]`

    Setting Values:
    For objects: `json["key"] = Value(123);`
    For arrays: `json.Append(Value(123));`

    Type Conversion:
    Use `json.AsXXX()` to retrieve values of specific types.

    Iterators:
    For arrays: `for (auto& item : json.GetArray())`
    For objects: `for (auto& item : json.GetObject())`

    Outputting JSON
    To output the JSON object as a string:
    Without indentation: `json.ToString()`
    With indentation: `json.ToString(true)`

    Example:
    std::cout << json << std::endl; // Outputs JSON string with indentation

    // generate json
    Value root;
    root.SetObject();
    root["name"] = Value("John");
    root["age"] = Value(30);
    root["married"] = Value(false);
    root["children"].SetArray();
    root["children"].Append(Value("Anna"));
    root["children"].Append(Value("Bob"));
    std::cout << root.ToString(true) << std::endl;

    @warning The JSON file being parsed should not be excessively large,
        as the entire string is loaded into memory during parsing.
*/


#include "json/json_tool.hpp"

#ifdef BRE_TEST
#include "json/test_json.hpp"
#endif
