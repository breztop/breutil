#include <benchmark/benchmark.h>

#include <boost/json.hpp>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "breutil/json.hpp"

#ifdef ENABLE_JSONCPP
#include <json/json.h>
#endif

#ifdef ENABLE_SIMDJSON
#include <simdjson.h>
#endif

namespace {

// 测试数据生成器
class TestDataGenerator {
public:
    static std::string generateSimpleJson() {
        return R"({"name": "John", "age": 30, "married": false, "children": ["Anna", "Bob"]})";
    }

    static std::string generateComplexJson() {
        return R"(
        [
    "JSON Test Pattern pass1",
    {"object with 1 member":["array with 1 element"]},
    {},
    [],
    -42,
    true,
    false,
    null,
    {
        "integer": 1234567890,
        "real": -9876.543210,
        "e": 0.123456789e-12,
        "E": 1.234567890E+34,
        "":  23456789012E66,
        "zero": 0,
        "one": 1,
        "space": " ",
        "quote": "\"",
        "backslash": "\\",
        "controls": "\b\f\n\r\t",
        "slash": "/ & \/",
        "alpha": "abcdefghijklmnopqrstuvwyz",
        "ALPHA": "ABCDEFGHIJKLMNOPQRSTUVWYZ",
        "digit": "0123456789",
        "0123456789": "digit",
        "special": "`1~!@#$%^&*()_+-={':[,]}|;.</>?",
        "hex": "\u0123\u4567\u89AB\uCDEF\uabcd\uef4A",
        "true": true,
        "false": false,
        "null": null,
        "array":[  ],
        "object":{  },
        "address": "50 St. James Street",
        "url": "http://www.JSON.org/",
        "comment": "// /* <!-- --",
        "# -- --> */": " ",
        " s p a c e d " :[1,2 , 3

,

4 , 5        ,          6           ,7        ],"compact":[1,2,3,4,5,6,7],
        "jsontext": "{\"object with 1 member\":[\"array with 1 element\"]}",
        "quotes": "&#34; \u0022 %22 0x22 034 &#x22;",
        "\/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"
: "A key can be any string"
    },
    0.5 ,98.6
,
99.44
,

1066,
1e1,
0.1e1,
1e-1,
1e00,2e+00,2e-00
,"rosebud"]
        )";
    }

    static std::string generateLargeArrayJson(size_t size) {
        std::string result = "[";
        for (size_t i = 0; i < size; ++i) {
            if (i > 0) result += ",";
            result += std::to_string(i);
        }
        result += "]";
        return result;
    }

    static std::string generateLargeObjectJson(size_t size) {
        std::string result = "{";
        for (size_t i = 0; i < size; ++i) {
            if (i > 0) result += ",";
            result += "\"key" + std::to_string(i) + "\": " + std::to_string(i);
        }
        result += "}";
        return result;
    }
};

template <typename Json, typename Parse>
void benchmarkParse(benchmark::State& state, const Json& json, Parse&& parse) {
    for (auto _ : state) {
        try {
            parse(json);
        } catch (const std::exception&) {
            state.SkipWithError("Parse failed");
            break;
        }
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(json.size()));
}

// =============== JSON 解析性能测试 ===============

// bre::json 解析简单 JSON
static void BM_BreJson_Parse_Simple(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateSimpleJson();

    for (auto _ : state) {
        try {
            bre::json::Value root = bre::json::Parser::parse(jsonStr, true);
            benchmark::DoNotOptimize(root);
        } catch (const std::exception& e) {
            state.SkipWithError("Parse failed");
        }
    }
}
BENCHMARK(BM_BreJson_Parse_Simple);

// boost::json 解析简单 JSON
static void BM_BoostJson_Parse_Simple(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateSimpleJson();

    for (auto _ : state) {
        try {
            boost::json::value root = boost::json::parse(jsonStr);
            benchmark::DoNotOptimize(root);
        } catch (const std::exception& e) {
            state.SkipWithError("Parse failed");
        }
    }
}
BENCHMARK(BM_BoostJson_Parse_Simple);

#ifdef ENABLE_JSONCPP
// jsoncpp 解析简单 JSON
static void BM_JsonCpp_Parse_Simple(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateSimpleJson();

    for (auto _ : state) {
        try {
            Json::Value root;
            Json::Reader reader;
            bool success = reader.parse(jsonStr, root);
            if (!success) {
                state.SkipWithError("Parse failed");
            }
            benchmark::DoNotOptimize(root);
        } catch (const std::exception& e) {
            state.SkipWithError("Parse failed");
        }
    }
}
BENCHMARK(BM_JsonCpp_Parse_Simple);
#endif

// bre::json 解析复杂 JSON
static void BM_BreJson_Parse_Complex(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateComplexJson();

    for (auto _ : state) {
        try {
            bre::json::Value root = bre::json::Parser::parse(jsonStr, true);
            benchmark::DoNotOptimize(root);
        } catch (const std::exception& e) {
            state.SkipWithError("Parse failed");
        }
    }
}
BENCHMARK(BM_BreJson_Parse_Complex);

// boost::json 解析复杂 JSON
static void BM_BoostJson_Parse_Complex(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateComplexJson();

    for (auto _ : state) {
        try {
            boost::json::value root = boost::json::parse(jsonStr);
            benchmark::DoNotOptimize(root);
        } catch (const std::exception& e) {
            state.SkipWithError("Parse failed");
        }
    }
}
BENCHMARK(BM_BoostJson_Parse_Complex);

#ifdef ENABLE_JSONCPP
// jsoncpp 解析复杂 JSON
static void BM_JsonCpp_Parse_Complex(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateComplexJson();

    for (auto _ : state) {
        try {
            Json::Value root;
            Json::Reader reader;
            bool success = reader.parse(jsonStr, root);
            if (!success) {
                state.SkipWithError("Parse failed");
            }
            benchmark::DoNotOptimize(root);
        } catch (const std::exception& e) {
            state.SkipWithError("Parse failed");
        }
    }
}
BENCHMARK(BM_JsonCpp_Parse_Complex);
#endif

// bre::json 解析大数组
static void BM_BreJson_Parse_LargeArray(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateLargeArrayJson(state.range(0));

    for (auto _ : state) {
        try {
            bre::json::Value root = bre::json::Parser::parse(jsonStr, true);
            benchmark::DoNotOptimize(root);
        } catch (const std::exception& e) {
            state.SkipWithError("Parse failed");
        }
    }
}
BENCHMARK(BM_BreJson_Parse_LargeArray)->Range(100, 10000);

// boost::json 解析大数组
static void BM_BoostJson_Parse_LargeArray(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateLargeArrayJson(state.range(0));

    for (auto _ : state) {
        try {
            boost::json::value root = boost::json::parse(jsonStr);
            benchmark::DoNotOptimize(root);
        } catch (const std::exception& e) {
            state.SkipWithError("Parse failed");
        }
    }
}
BENCHMARK(BM_BoostJson_Parse_LargeArray)->Range(100, 10000);

#ifdef ENABLE_JSONCPP
// jsoncpp 解析大数组
static void BM_JsonCpp_Parse_LargeArray(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateLargeArrayJson(state.range(0));

    for (auto _ : state) {
        try {
            Json::Value root;
            Json::Reader reader;
            bool success = reader.parse(jsonStr, root);
            if (!success) {
                state.SkipWithError("Parse failed");
            }
            benchmark::DoNotOptimize(root);
        } catch (const std::exception& e) {
            state.SkipWithError("Parse failed");
        }
    }
}
BENCHMARK(BM_JsonCpp_Parse_LargeArray)->Range(100, 10000);
#endif

// =============== JSON 生成性能测试 ===============

// bre::json 生成简单对象
static void BM_BreJson_Generate_Simple(benchmark::State& state) {
    for (auto _ : state) {
        bre::json::Value root;
        root.SetObject();
        root["name"] = bre::json::Value("John");
        root["age"] = bre::json::Value(30);
        root["married"] = bre::json::Value(false);
        root["children"].SetArray();
        root["children"].Append(bre::json::Value("Anna"));
        root["children"].Append(bre::json::Value("Bob"));

        std::string result = root.ToString();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_BreJson_Generate_Simple);

// boost::json 生成简单对象
static void BM_BoostJson_Generate_Simple(benchmark::State& state) {
    for (auto _ : state) {
        boost::json::object root;
        root["name"] = "John";
        root["age"] = 30;
        root["married"] = false;
        root["children"] = boost::json::array{"Anna", "Bob"};

        std::string result = boost::json::serialize(root);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_BoostJson_Generate_Simple);

#ifdef ENABLE_JSONCPP
// jsoncpp 生成简单对象
static void BM_JsonCpp_Generate_Simple(benchmark::State& state) {
    for (auto _ : state) {
        Json::Value root;
        root["name"] = "John";
        root["age"] = 30;
        root["married"] = false;
        root["children"] = Json::Value(Json::arrayValue);
        root["children"].append("Anna");
        root["children"].append("Bob");

        Json::StreamWriterBuilder builder;
        std::string result = Json::writeString(builder, root);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_JsonCpp_Generate_Simple);
#endif

// bre::json 生成大对象
static void BM_BreJson_Generate_LargeObject(benchmark::State& state) {
    for (auto _ : state) {
        bre::json::Value root;
        root.SetObject();

        for (int i = 0; i < state.range(0); ++i) {
            std::string key = "key" + std::to_string(i);
            root[key] = bre::json::Value(i);
        }

        std::string result = root.ToString();
        benchmark::DoNotOptimize(result);
    }
    // 为复杂度分析设置 N
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_BreJson_Generate_LargeObject)->Range(100, 10000)->Complexity();

// boost::json 生成大对象
static void BM_BoostJson_Generate_LargeObject(benchmark::State& state) {
    for (auto _ : state) {
        boost::json::object root;

        for (int i = 0; i < state.range(0); ++i) {
            std::string key = "key" + std::to_string(i);
            root[key] = i;
        }

        std::string result = boost::json::serialize(root);
        benchmark::DoNotOptimize(result);
    }
    // 为复杂度分析设置 N
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_BoostJson_Generate_LargeObject)->Range(100, 10000)->Complexity();

#ifdef ENABLE_JSONCPP
// jsoncpp 生成大对象
static void BM_JsonCpp_Generate_LargeObject(benchmark::State& state) {
    for (auto _ : state) {
        Json::Value root;

        for (int i = 0; i < state.range(0); ++i) {
            std::string key = "key" + std::to_string(i);
            root[key] = i;
        }

        Json::StreamWriterBuilder builder;
        std::string result = Json::writeString(builder, root);
        benchmark::DoNotOptimize(result);
    }
    // 为复杂度分析设置 N
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_JsonCpp_Generate_LargeObject)->Range(100, 10000)->Complexity();
#endif

// =============== 对象操作性能测试 ===============

// bre::json 对象访问
static void BM_BreJson_ObjectAccess(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateComplexJson();
    const bre::json::Value root = bre::json::Parser::parse(jsonStr, true);

    for (auto _ : state) {
        // 访问数组第二个元素中的对象
        const auto& objectWithMember = root.At(1).At("object with 1 member");
        const auto& firstElement = objectWithMember.At(0).AsString();

        // 访问数组第九个元素的复杂对象
        const auto& complexObj = root.At(8);
        const auto integer = complexObj.At("integer").AsInt();
        const auto realNum = complexObj.At("real").AsDouble();
        const auto& address = complexObj.At("address").AsString();
        const auto& url = complexObj.At("url").AsString();

        benchmark::DoNotOptimize(firstElement);
        benchmark::DoNotOptimize(integer);
        benchmark::DoNotOptimize(realNum);
        benchmark::DoNotOptimize(address);
        benchmark::DoNotOptimize(url);
    }
}
BENCHMARK(BM_BreJson_ObjectAccess);

// boost::json 对象访问
static void BM_BoostJson_ObjectAccess(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateComplexJson();
    boost::json::value root = boost::json::parse(jsonStr);

    for (auto _ : state) {
        // 访问数组第二个元素中的对象
        auto& objectWithMember = root.as_array()[1].as_object()["object with 1 member"].as_array();
        auto firstElement = objectWithMember[0].as_string();

        // 访问数组第九个元素的复杂对象
        auto& complexObj = root.as_array()[8].as_object();
        auto integer = complexObj["integer"].as_int64();
        auto realNum = complexObj["real"].as_double();
        auto address = complexObj["address"].as_string();
        auto url = complexObj["url"].as_string();

        benchmark::DoNotOptimize(firstElement);
        benchmark::DoNotOptimize(integer);
        benchmark::DoNotOptimize(realNum);
        benchmark::DoNotOptimize(address);
        benchmark::DoNotOptimize(url);
    }
}
BENCHMARK(BM_BoostJson_ObjectAccess);

#ifdef ENABLE_JSONCPP
// jsoncpp 对象访问
static void BM_JsonCpp_ObjectAccess(benchmark::State& state) {
    std::string jsonStr = TestDataGenerator::generateComplexJson();
    Json::Value root;
    Json::Reader reader;
    reader.parse(jsonStr, root);

    for (auto _ : state) {
        // 访问数组第二个元素中的对象
        auto& objectWithMember = root[1]["object with 1 member"];
        auto firstElement = objectWithMember[0].asString();

        // 访问数组第九个元素的复杂对象
        auto& complexObj = root[8];
        auto integer = complexObj["integer"].asInt();
        auto realNum = complexObj["real"].asDouble();
        auto address = complexObj["address"].asString();
        auto url = complexObj["url"].asString();

        benchmark::DoNotOptimize(firstElement);
        benchmark::DoNotOptimize(integer);
        benchmark::DoNotOptimize(realNum);
        benchmark::DoNotOptimize(address);
        benchmark::DoNotOptimize(url);
    }
}
BENCHMARK(BM_JsonCpp_ObjectAccess);
#endif

// =============== 数组操作性能测试 ===============

// bre::json 数组追加
static void BM_BreJson_ArrayAppend(benchmark::State& state) {
    for (auto _ : state) {
        bre::json::Value arr;
        arr.SetArray();

        for (int i = 0; i < state.range(0); ++i) {
            arr.Append(bre::json::Value(i));
        }

        benchmark::DoNotOptimize(arr);
    }
}
BENCHMARK(BM_BreJson_ArrayAppend)->Range(100, 10000);

// boost::json 数组追加
static void BM_BoostJson_ArrayAppend(benchmark::State& state) {
    for (auto _ : state) {
        boost::json::array arr;

        for (int i = 0; i < state.range(0); ++i) {
            arr.push_back(i);
        }

        benchmark::DoNotOptimize(arr);
    }
}
BENCHMARK(BM_BoostJson_ArrayAppend)->Range(100, 10000);

#ifdef ENABLE_JSONCPP
// jsoncpp 数组追加
static void BM_JsonCpp_ArrayAppend(benchmark::State& state) {
    for (auto _ : state) {
        Json::Value arr(Json::arrayValue);

        for (int i = 0; i < state.range(0); ++i) {
            arr.append(i);
        }

        benchmark::DoNotOptimize(arr);
    }
}
BENCHMARK(BM_JsonCpp_ArrayAppend)->Range(100, 10000);
#endif

// bre::json 数组访问
static void BM_BreJson_ArrayAccess(benchmark::State& state) {
    bre::json::Value arr;
    arr.SetArray();
    for (int i = 0; i < 1000; ++i) {
        arr.Append(bre::json::Value(i));
    }

    for (auto _ : state) {
        int sum = 0;
        for (size_t i = 0; i < arr.Size(); ++i) {
            sum += arr[i].AsInt();
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_BreJson_ArrayAccess);

// boost::json 数组访问
static void BM_BoostJson_ArrayAccess(benchmark::State& state) {
    boost::json::array arr;
    for (int i = 0; i < 1000; ++i) {
        arr.push_back(i);
    }

    for (auto _ : state) {
        int sum = 0;
        for (size_t i = 0; i < arr.size(); ++i) {
            sum += arr[i].as_int64();
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_BoostJson_ArrayAccess);

#ifdef ENABLE_JSONCPP
// jsoncpp 数组访问
static void BM_JsonCpp_ArrayAccess(benchmark::State& state) {
    Json::Value arr(Json::arrayValue);
    for (int i = 0; i < 1000; ++i) {
        arr.append(i);
    }

    for (auto _ : state) {
        int sum = 0;
        for (Json::ArrayIndex i = 0; i < arr.size(); ++i) {
            sum += arr[i].asInt();
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_JsonCpp_ArrayAccess);
#endif

// =============== 内存使用测试 ===============

// bre::json 内存使用 - 大量小对象
static void BM_BreJson_Memory_SmallObjects(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<bre::json::Value> objects;
        objects.reserve(state.range(0));

        for (int i = 0; i < state.range(0); ++i) {
            bre::json::Value obj;
            obj.SetObject();
            obj["id"] = bre::json::Value(i);
            obj["name"] = bre::json::Value("Object" + std::to_string(i));
            objects.push_back(std::move(obj));
        }

        benchmark::DoNotOptimize(objects);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_BreJson_Memory_SmallObjects)->Range(100, 10000)->Complexity();

// boost::json 内存使用 - 大量小对象
static void BM_BoostJson_Memory_SmallObjects(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<boost::json::object> objects;
        objects.reserve(state.range(0));

        for (int i = 0; i < state.range(0); ++i) {
            boost::json::object obj;
            obj["id"] = i;
            obj["name"] = "Object" + std::to_string(i);
            objects.push_back(std::move(obj));
        }

        benchmark::DoNotOptimize(objects);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_BoostJson_Memory_SmallObjects)->Range(100, 10000)->Complexity();

#ifdef ENABLE_JSONCPP
// jsoncpp 内存使用 - 大量小对象
static void BM_JsonCpp_Memory_SmallObjects(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<Json::Value> objects;
        objects.reserve(state.range(0));

        for (int i = 0; i < state.range(0); ++i) {
            Json::Value obj;
            obj["id"] = i;
            obj["name"] = "Object" + std::to_string(i);
            objects.push_back(std::move(obj));
        }

        benchmark::DoNotOptimize(objects);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_JsonCpp_Memory_SmallObjects)->Range(100, 10000)->Complexity();
#endif

#ifdef ENABLE_SIMDJSON
static void BM_SimdJson_Parse_Simple(benchmark::State& state) {
    const simdjson::padded_string json = TestDataGenerator::generateSimpleJson();
    simdjson::dom::parser parser;
    benchmarkParse(state, json, [&parser](const auto& input) {
        auto root = parser.parse(input);
        benchmark::DoNotOptimize(root);
    });
}
BENCHMARK(BM_SimdJson_Parse_Simple);

static void BM_SimdJson_Parse_Complex(benchmark::State& state) {
    const simdjson::padded_string json = TestDataGenerator::generateComplexJson();
    simdjson::dom::parser parser;
    benchmarkParse(state, json, [&parser](const auto& input) {
        auto root = parser.parse(input);
        benchmark::DoNotOptimize(root);
    });
}
BENCHMARK(BM_SimdJson_Parse_Complex);

static void BM_SimdJson_Parse_LargeArray(benchmark::State& state) {
    const simdjson::padded_string json =
        TestDataGenerator::generateLargeArrayJson(state.range(0));
    simdjson::dom::parser parser;
    benchmarkParse(state, json, [&parser](const auto& input) {
        auto root = parser.parse(input);
        benchmark::DoNotOptimize(root);
    });
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_SimdJson_Parse_LargeArray)->Range(100, 10000)->Complexity();

static void BM_SimdJson_ObjectAccess(benchmark::State& state) {
    const simdjson::padded_string json = TestDataGenerator::generateComplexJson();
    simdjson::dom::parser parser;
    const simdjson::dom::element root = parser.parse(json);

    for (auto _ : state) {
        const std::string_view firstElement =
            root.at(1)["object with 1 member"].at(0).get_string();
        const auto complexObject = root.at(8);
        const int64_t integer = complexObject["integer"].get_int64();
        const double realNumber = complexObject["real"].get_double();
        const std::string_view address = complexObject["address"].get_string();
        const std::string_view url = complexObject["url"].get_string();

        benchmark::DoNotOptimize(firstElement);
        benchmark::DoNotOptimize(integer);
        benchmark::DoNotOptimize(realNumber);
        benchmark::DoNotOptimize(address);
        benchmark::DoNotOptimize(url);
    }
}
BENCHMARK(BM_SimdJson_ObjectAccess);

static void BM_SimdJson_ArrayAccess(benchmark::State& state) {
    const simdjson::padded_string json =
        TestDataGenerator::generateLargeArrayJson(1000);
    simdjson::dom::parser parser;
    const simdjson::dom::array array = parser.parse(json).get_array();

    for (auto _ : state) {
        int64_t sum = 0;
        for (const simdjson::dom::element value : array) {
            sum += value.get_int64();
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_SimdJson_ArrayAccess);
#endif  // ENABLE_SIMDJSON

}  // namespace

BENCHMARK_MAIN();
