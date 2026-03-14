#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "../json.hpp"


using namespace bre;
using namespace bre::json;
static void test_generete_object() {
    Value jsonObject;
    jsonObject.SetObject();
    jsonObject["int"] = Value(10);
    jsonObject["string"] = Value("hello");
    jsonObject["bool"] = Value(true);
    jsonObject["null"] = Value();

    std::cout << jsonObject["int"] << std::endl;
    std::cout << jsonObject["string"] << std::endl;
    std::cout << jsonObject["bool"] << std::endl;
    std::cout << jsonObject["null"] << std::endl;

    std::cout << jsonObject.ToString(true) << "\n\n";
}

static void test_generete_array() {
    Value jsonArray;
    jsonArray.SetArray();
    jsonArray.Append(Value(10));
    jsonArray.Append(Value("hello"));
    jsonArray.Append(Value(true));
    jsonArray.Append(Value());
    for (size_t i = 0; i < jsonArray.Size(); i++) {
        std::cout << jsonArray[i] << std::endl;
    }
    std::cout << jsonArray.ToString(true) << "\n\n";
}

static void test_generate_int() {
    Value jsonInt;
    jsonInt.SetInt(123457890);
    std::cout << jsonInt.AsInt() << "\n\n";
}

static void test_generate_bool_null() {
    Value jsonBool;
    jsonBool.SetBool(true);

    Value jsonNull;
    jsonNull.SetNull();
    std::cout << jsonBool.AsBool() << std::endl;

    std::cout << jsonNull << "\n\n";
    std::cout << (jsonNull.IsNull() ? "null" : "not null") << std::endl;
}

static void test_generate_str() {
    // 创建一个 JSON 对象
    Value root;
    root.SetObject();
    root["name"] = Value("John");
    root["age"] = Value(30);
    root["married"] = Value(false);
    root["children"].SetArray();
    root["children"].Append(Value("Anna"));
    root["children"].Append(Value("Bob"));

    try {
        std::string jsonString = Generator::generate(root);
        std::cout << jsonString << std::endl;
    } catch (const JsonParseException& e) {
        std::cerr << "Generation error: " << e.what() << std::endl;
    }
}

static void test_generete_complex() {
    std::string jsonString = R"(
[
  "JSON Test Pattern pAss1",
  {
    "object with 1 member": [
      "array with 1 element"
    ]
  },
  {},
  [],
  -42,
  true,
  false,
  null,
  {
    "e": 0.000000,
    "": 234567890.000000,
    "hex": "ģ䕧覫췯ꯍ",
    "real": -9876.543210,
    "url": "http://www.JSON.org/",
    "integer": 1234567890,
    "digit": "0123456789",
    "quote": "\"",
    "null": null,
    "E": 12345.678900,
    "array": [],
    "zero": 0,
    "space": " ",
    "compact": [
      1,
      2,
      3,
      4,
      5,
      6,
      7
    ],
    "special": "`1~!@#$%^&*()_+-={':[]}|;.</>?",
    "one": 1,
    "0123456789": "digit",
    "backslAsh": "\\",
    "ALPHA": "ABCDEFGHIJKLMNOPQRSTUVWYZ",
    "controls": "\b\f\n\r\t",
    "quotes": "&#34; \" %22 0x22 034 &#x22;",
    "slAsh": "/ & /",
    " s p a c e d ": [
      1,
      2,
      3,
      4,
      5,
      6,
      7
    ],
    "alpha": "abcdefghijklmnopqrstuvwyz",
    "true": true,
    "false": false,
    "object": {},
    "address": "50 St. James Street",
    "comment": "// /* <!-- --",
    "# -- --> */": " ",
    "jsontext": "{\"object with 1 member\":[\"array with 1 element\"]}",
    "/\\\"쫾몾ꮘﳞ볚\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?": "A key can be any string"
  },
  0.500000,
  98.600000,
  99.440000,
  1066,
  10,
  1,
  0.100000,
  1,
  2,
  2,
  "rosebud"
]
)";
    Value Json = Parser::parse(jsonString);
    std::cout << Json << std::endl;

    // 获取compact 数组
    Value compact = Json[8]["jsontext"];
    std::cout << compact << std::endl;

    // 通过建立生成上面的Json
    Value root;
    root.SetArray();
    root.Append(Value("JSON Test Pattern pAss1"));
    Value obj;
    obj.SetObject();
    obj["object with 1 member"].SetArray();
    obj["object with 1 member"].Append(Value("array with 1 element"));
    root.Append(obj);
    root.Append(Value::Object{});
    root.Append(Value::Array{});
    root.Append(Value(-42));
    root.Append(Value(true));
    root.Append(Value(false));
    root.Append(Value());
    Value obj2;
    obj2.SetObject();
    obj2["e"] = Value(0.000000);
    obj2["quote"] = Value("\"");
    obj2[""] = Value(234567890.0);
    obj2["integer"] = Value(1234567890);
    obj2["real"] = Value(-9876.54321);
    obj2["E"] = Value(1.23456789e+4);
    obj2["zero"] = Value(0);
    obj2["one"] = Value(1);
    obj2["space"] = Value(" ");
    obj2["backslAsh"] = Value("\\");
    obj2["controls"] = Value("\b\f\n\r\t");
    obj2["slAsh"] = Value("/ & /");
    obj2["alpha"] = Value("abcdefghijklmnopqrstuvwyz");
    obj2["ALPHA"] = Value("ABCDEFGHIJKLMNOPQRSTUVWYZ");
    obj2["digit"] = Value("0123456789");
    obj2["0123456789"] = Value("digit");
    obj2["special"] = Value("`1~!@#$%^&*()_+-={':[]}|;.</>?");
    obj2["hex"] = Value("ģ䕧覫췯ꯍ");
    obj2["true"] = Value(true);
    obj2["false"] = Value(false);
    obj2["null"] = Value();
    obj2["array"].SetArray();
    obj2["object"].SetObject();
    obj2["address"] = Value("50 St. James Street");
    obj2["url"] = Value("http://www.JSON.org/");
    obj2["comment"] = Value("// /* <!-- --");
    obj2["# -- --> */"] = Value(" ");
    obj2[" s p a c e d "].SetArray();
    for (int i = 0; i < 7; ++i) {
        obj2[" s p a c e d "].Append(Value(i + 1));
    }
    obj2["compact"].SetArray();
    for (int i = 0; i < 7; ++i) {
        obj2["compact"].Append(Value(i + 1));
    }
    obj2["jsontext"] = Value("{\"object with 1 member\":[\"array with 1 element\"]}");
    obj2["quotes"] = Value("&#34; \" %22 0x22 034 &#x22;");
    obj2["/\\\"쫾몾ꮘﳞ볚\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"] =
        Value("A key can be any string");
    root.Append(obj2);
    root.Append(Value(0.5));
    root.Append(Value(98.6));
    root.Append(Value(99.44));
    root.Append(Value(1066));
    root.Append(Value(10));
    root.Append(Value(1));
    root.Append(Value(0.1));
    root.Append(Value(1));
    root.Append(Value(2));
    root.Append(Value(2));
    root.Append(Value("rosebud"));
    std::cout << "\n\n";
    std::cout << root << std::endl;
    assert(Json == root);
}

// =======================下面为解析测试=======================

static void test_parse_jsoncpp_testfilejson() {
    std::string testDir = "test/data";
    std::vector<std::string> fileNames{"test/data/fail_invalid_quote.json",
                                       "test/data/fail_strict_comment_01.json",
                                       "test/data/fail_strict_comment_02.json",
                                       "test/data/fail_strict_comment_03.json",
                                       "test/data/fail_test_array_01.json",
                                       "test/data/fail_test_array_02.json",
                                       "test/data/fail_test_object_01.json",
                                       "test/data/fail_test_object_02.json"
                                       // ,"test/data/fail_test_stack_limit.json"
                                       ,
                                       "test/data/legacy_test_array_01.json",
                                       "test/data/legacy_test_array_02.json",
                                       "test/data/legacy_test_array_03.json",
                                       "test/data/legacy_test_array_04.json",
                                       "test/data/legacy_test_array_05.json",
                                       "test/data/legacy_test_array_06.json",
                                       "test/data/legacy_test_array_07.json",
                                       "test/data/legacy_test_bAsic_01.json",
                                       "test/data/legacy_test_bAsic_02.json",
                                       "test/data/legacy_test_bAsic_03.json",
                                       "test/data/legacy_test_bAsic_04.json",
                                       "test/data/legacy_test_bAsic_05.json",
                                       "test/data/legacy_test_bAsic_06.json",
                                       "test/data/legacy_test_bAsic_07.json",
                                       "test/data/legacy_test_bAsic_08.json",
                                       "test/data/legacy_test_bAsic_09.json",
                                       "test/data/legacy_test_comment_00.json",
                                       "test/data/legacy_test_comment_01.json",
                                       "test/data/legacy_test_comment_02.json",
                                       "test/data/legacy_test_complex_01.json",
                                       "test/data/legacy_test_integer_01.json",
                                       "test/data/legacy_test_integer_02.json",
                                       "test/data/legacy_test_integer_03.json",
                                       "test/data/legacy_test_integer_04.json",
                                       "test/data/legacy_test_integer_05.json",
                                       "test/data/legacy_test_integer_06_64bits.json",
                                       "test/data/legacy_test_integer_07_64bits.json",
                                       "test/data/legacy_test_integer_08_64bits.json",
                                       "test/data/legacy_test_large_01.json",
                                       "test/data/legacy_test_object_01.json",
                                       "test/data/legacy_test_object_02.json",
                                       "test/data/legacy_test_object_03.json",
                                       "test/data/legacy_test_object_04.json",
                                       "test/data/legacy_test_preserve_comment_01.json",
                                       "test/data/legacy_test_real_01.json",
                                       "test/data/legacy_test_real_02.json",
                                       "test/data/legacy_test_real_03.json",
                                       "test/data/legacy_test_real_04.json",
                                       "test/data/legacy_test_real_05.json",
                                       "test/data/legacy_test_real_06.json",
                                       "test/data/legacy_test_real_07.json",
                                       "test/data/legacy_test_real_08.json",
                                       "test/data/legacy_test_real_09.json",
                                       "test/data/legacy_test_real_10.json",
                                       "test/data/legacy_test_real_11.json",
                                       "test/data/legacy_test_real_12.json",
                                       "test/data/legacy_test_real_13.json",
                                       "test/data/legacy_test_string_01.json",
                                       "test/data/legacy_test_string_02.json",
                                       "test/data/legacy_test_string_03.json",
                                       "test/data/legacy_test_string_04.json",
                                       "test/data/legacy_test_string_05.json",
                                       "test/data/legacy_test_string_unicode_01.json",
                                       "test/data/legacy_test_string_unicode_02.json",
                                       "test/data/legacy_test_string_unicode_03.json",
                                       "test/data/legacy_test_string_unicode_04.json",
                                       "test/data/legacy_test_string_unicode_05.json"};


    for (auto& jsonfile : fileNames) {
        std::ifstream file(jsonfile);
        if (!file.is_open()) {
            std::cerr << "Failed to open file" << std::endl;
            return;
        }

        std::string jsonString((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
        file.close();
        std::cout << "test file: " << jsonfile << "\n";
        // std::cout << jsonString << "\n\n===========\n\n";
        // 解析 JSON 字符串
        try {
            Value root = json::Parser::parse(jsonString);

            if (root.Size() == 0) {
                std::cout << root << std::endl;
            } else {
                std::cout << "size: " << root.Size() << std::endl;
            }
        } catch (const JsonParseException& e) {
            std::string msg = std::string("Parse error: ") + e.what();
            std::cerr << msg << std::endl;
        } catch (const std::exception& e) {
            std::string msg = std::string("Parse error: ") + e.what();
            std::cerr << msg << std::endl;
        }
        std::cout << "\n";
    }
}

// 测试尾随逗号，应该解析失败，但是默认成功
static void test_parse_TailComma() {
    std::string testDir = "test/data";
    std::vector<std::string> fileNames{
        "test/data/test_object_05.json",
        "test/data/test_array_08.json",
    };


    for (auto& jsonfile : fileNames) {
        std::ifstream file(jsonfile);
        if (!file.is_open()) {
            std::cerr << "Failed to open file" << std::endl;
            return;
        }

        std::string jsonString((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
        file.close();
        std::cout << "test file: " << jsonfile << "\n";
        // std::cout << jsonString << "\n\n===========\n\n";
        // 解析 JSON 字符串
        try {
            Value root = Parser::parse(jsonString);
            std::cout << root << std::endl;
        } catch (const JsonParseException& e) {
            std::string msg = std::string("Parse error: ") + e.what();
            std::cerr << msg << std::endl;
        } catch (const std::exception& e) {
            std::string msg = std::string("Parse error: ") + e.what();
            std::cerr << msg << std::endl;
        }
        std::cout << "\n";
    }
}

// 测试 jsonchecker 的测试用例
static void test_parse_jsonchecker() {
    std::string testDir = "test/jsonchecker";
    std::vector<std::string> fileNames;

    for (int i = 1; i <= 33; i++) {
        std::string filename = "test/jsonchecker/fail" + std::to_string(i) + ".json";
        fileNames.push_back(filename);
    }

    for (int i = 0; i <= 3; ++i) {
        std::string filename = "test/jsonchecker/pass" + std::to_string(i) + ".json";
        fileNames.push_back(filename);
    }

    for (auto& jsonfile : fileNames) {
        std::ifstream file(jsonfile);
        if (!file.is_open()) {
            std::cerr << "Failed to open file" << std::endl;
            return;
        }

        std::string jsonString((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
        file.close();
        std::cout << "test file: " << jsonfile << "\n";
        // std::cout << jsonString << "\n\n===========\n\n";
        // 解析 JSON 字符串
        try {
            Value root = Parser::parse(jsonString);
            std::cout << root << std::endl;
        } catch (const JsonParseException& e) {
            std::string msg = std::string("Parse error: ") + e.what();
            std::cerr << msg << std::endl;
        } catch (const std::exception& e) {
            std::string msg = std::string("Parse error: ") + e.what();
            std::cerr << msg << std::endl;
        }
        std::cout << "\n";
    }
}

void test_Iterator() {
    std::string jsonString = R"(
    "b站": "微风中的快乐"
)";
    try {
        Value root = Parser::parse(jsonString);
        for (auto& it : root) {
            std::cout << it << std::endl;
        }
    } catch (const JsonParseException& e) {
        std::string msg = std::string("Parse error: ") + e.what();
        std::cerr << msg << std::endl;
    } catch (const std::exception& e) {
        std::string msg = std::string("Parse error: ") + e.what();
        std::cerr << msg << std::endl;
    }
}


int test_json() {
    // 测试解析
    test_parse_jsoncpp_testfilejson();
    test_parse_jsonchecker();
    test_parse_TailComma();

    // 测试生成
    test_generete_object();
    test_generete_array();
    test_generate_int();
    test_generate_bool_null();
    test_generate_str();
    test_generete_complex();
    test_Iterator();

    return 0;
}
