#include <algorithm>
#include <any>
#include <boost/test/unit_test.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "breutil/config.hpp"

using namespace bre;

class a_object {
public:
    a_object(int v = 0) : value(v) {}
    void one() { std::cout << "one: value = " << value << std::endl; }

    void two() { std::cout << "two: value = " << value << std::endl; }
    int value;
};


BOOST_AUTO_TEST_SUITE(ConfigTestSuite)

BOOST_AUTO_TEST_CASE(test_register_set_get_basic) {
    auto config = Config::Instance();
    config->Reset();

    config->Register<int>("capture.fps", 30);
    config->Register<std::string>("capture.codec", "h264");

    auto old_fps = config->Set<int>("capture.fps", 120);
    BOOST_CHECK(old_fps);
    BOOST_CHECK_EQUAL(old_fps.value(), 30);

    auto fps_opt = config->Get<int>("capture.fps");
    BOOST_CHECK(fps_opt);
    BOOST_CHECK_EQUAL(fps_opt.value(), 120);

    auto codec_opt = config->Get<std::string>("capture.codec");
    BOOST_CHECK(codec_opt);
    BOOST_CHECK_EQUAL(codec_opt.value(), "h264");

    BOOST_CHECK(!config->Get<int>("unregistered.key"));
}

// BOOST_AUTO_TEST_CASE(test_validator_clamping_and_type_mismatch) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("capture.fps", [](bool is_set, int v) {
//         if (!is_set) return 30;
//         if (v < 1) return 1;
//         if (v > 500) return 500;
//         return v;
//     });

//     config->Set<int>("capture.fps", 800);
//     BOOST_CHECK_EQUAL(config->Get<int>("capture.fps").value(), 500);

//     config->Set<int>("capture.fps", -5);
//     BOOST_CHECK_EQUAL(config->Get<int>("capture.fps").value(), 1);

//     BOOST_CHECK_THROW(config->Get<std::string>("capture.fps"), std::bad_any_cast);
// }

// BOOST_AUTO_TEST_CASE(test_notify_change_callback) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("capture.fps", 30);

//     bool callback_fired = false;
//     int callback_value = 0;
//     config->NotifyChange("capture.fps", [&](std::any new_value) {
//         callback_fired = true;
//         callback_value = std::any_cast<int>(new_value);
//     });

//     config->Set<int>("capture.fps", 75);
//     BOOST_CHECK(callback_fired);
//     BOOST_CHECK_EQUAL(callback_value, 75);
// }

// BOOST_AUTO_TEST_CASE(test_to_json_and_from_json) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("capture.fps", 30);
//     config->Register<std::string>("capture.codec", "h264");
//     config->Register<bool>("capture.audio", true);

//     config->Set<int>("capture.fps", 60);
//     config->Set<std::string>("capture.codec", "vp9");

//     std::string json = config->ToJson(true);
//     BOOST_CHECK(json.find("\"capture\"") != std::string::npos);
//     BOOST_CHECK(json.find("\"fps\"") != std::string::npos);
//     BOOST_CHECK(json.find("vp9") != std::string::npos);

//     std::string new_json = R"({
//       "capture": {
//         "fps": 200,
//         "codec": "h265",
//         "audio": false
//       }
//     })";

//     config->FromJson(new_json);
//     BOOST_CHECK_EQUAL(config->Get<int>("capture.fps").value(), 200);
//     BOOST_CHECK_EQUAL(config->Get<std::string>("capture.codec").value(), "h265");
//     BOOST_CHECK_EQUAL(config->Get<bool>("capture.audio").value(), false);
// }

// BOOST_AUTO_TEST_CASE(test_non_serializable_shared_ptr) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<std::shared_ptr<int>>("data.ptr", std::make_shared<int>(123));

//     auto old_ptr = config->Set<std::shared_ptr<int>>("data.ptr", std::make_shared<int>(456));
//     BOOST_CHECK(!old_ptr);

//     auto ptr_v = config->Get<std::shared_ptr<int>>("data.ptr");
//     BOOST_CHECK(ptr_v);
//     BOOST_CHECK_EQUAL(*ptr_v.value(), 456);

//     std::string json = config->ToJson();
//     BOOST_CHECK(json.find("__non_serializable") != std::string::npos);
// }

// BOOST_AUTO_TEST_CASE(test_set_unregistered_key_throws) {
//     auto config = Config::Instance();
//     config->Reset();
//     BOOST_CHECK_THROW(config->Set<int>("not.exist", 1), std::invalid_argument);
// }

// BOOST_AUTO_TEST_CASE(test_string_type_and_re_registration_behavior) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<const char*>("capture", nullptr);
//     config->Register<int>("capture.fps", [](bool is_set, int v) {
//         if (!is_set) return 30;
//         if (v < 1) return 1;
//         if (v > 500) return 500;
//         return v;
//     });
//     config->Register<std::string>("capture.codec", [](bool is_set, const std::string& v) {
//         if (!is_set) return std::string("h264");
//         if (v == "h264" || v == "h265" || v == "vp9") return v;
//         return std::string("h264");
//     });

//     config->Set<std::string>("capture", "hello");
//     BOOST_CHECK_EQUAL(config->Get<std::string>("capture").value(), "hello");

//     config->Set("capture", "this value is to set session capture");
//     BOOST_CHECK_EQUAL(config->Get<std::string>("capture").value(),
//                       "this value is to set session capture");

//     config->Register<int>("capture.fps", [](bool, int v) {
//         return v < 0 ? 0 : v;
//     });
//     config->Set<int>("capture.fps", -5);
//     BOOST_CHECK_EQUAL(config->Get<int>("capture.fps").value(), 0);
// }

// BOOST_AUTO_TEST_CASE(test_reset_default_and_getallkeys_has) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<bool>("source_2.enable", true);
//     config->Register<std::string>("source_2.device", "/dev/video0");

//     BOOST_CHECK(config->Has("source_2.enable"));
//     BOOST_CHECK(config->Has("source_2.device"));
//     BOOST_CHECK(!config->Has("source_2.missing"));

//     auto keys = config->GetAllKeys();
//     BOOST_CHECK(std::find(keys.begin(), keys.end(), "source_2.enable") != keys.end());
//     BOOST_CHECK(std::find(keys.begin(), keys.end(), "source_2.device") != keys.end());

//     BOOST_CHECK_EQUAL(config->Get<bool>("source_2.enable").value(), true);
//     BOOST_CHECK_EQUAL(config->Get<std::string>("source_2.device").value(), "/dev/video0");

//     config->Set<bool>("source_2.enable", false);
//     BOOST_CHECK_EQUAL(config->Get<bool>("source_2.enable").value(), false);

//     config->Reset();
//     BOOST_CHECK_EQUAL(config->Get<bool>("source_2.enable").value(), true);
// }

// BOOST_AUTO_TEST_CASE(test_enum_registration_and_type_mismatch_runtime_error) {
//     auto config = Config::Instance();
//     config->Reset();

//     enum class TestEnum { VALUE_ONE, VALUE_TWO, VALUE_THREE };
//     config->Register<TestEnum>("enum.test", TestEnum::VALUE_TWO);

//     auto e = config->Get<TestEnum>("enum.test");
//     BOOST_CHECK(e);
//     BOOST_CHECK(e.value() == TestEnum::VALUE_TWO);

//     BOOST_CHECK_THROW(config->Get<std::string>("enum.test"), std::runtime_error);
// }

// BOOST_AUTO_TEST_CASE(test_from_json_with_nested_value_and_non_serializable_skip) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("capture.fps", 30);
//     config->Register<std::string>("capture.codec", "h264");
//     config->Register<bool>("capture.audio", true);
//     config->Register<std::shared_ptr<int>>("data.ptr", nullptr);

//     std::string json = R"({
//       "capture": {
//         "__value": "ignore",
//         "fps": 100,
//         "codec": "h265",
//         "audio": false
//       },
//       "data": {
//         "ptr": {"unexpected":123}
//       }
//     })";

//     config->FromJson(json);
//     BOOST_CHECK_EQUAL(config->Get<int>("capture.fps").value(), 100);
//     BOOST_CHECK_EQUAL(config->Get<std::string>("capture.codec").value(), "h265");
//     BOOST_CHECK_EQUAL(config->Get<bool>("capture.audio").value(), false);

//     std::string out = config->ToJson(true);
//     BOOST_CHECK(out.find("__non_serializable") != std::string::npos);
// }

// BOOST_AUTO_TEST_CASE(test_invalid_registration_key_parts_throw) {
//     auto config = Config::Instance();
//     config->Reset();

//     BOOST_CHECK_THROW(config->Register<int>("", 1), std::invalid_argument);
//     BOOST_CHECK_THROW(config->Register<int>("a..b", 1), std::invalid_argument);
//     BOOST_CHECK_THROW(config->Register<int>("a b", 1), std::invalid_argument);
//     BOOST_CHECK_THROW(config->Register<int>("__bad", 1), std::invalid_argument);
// }

// BOOST_AUTO_TEST_CASE(test_get_set_string_pointers_and_views) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<const char*>("path", "hello");
//     config->Register<std::string>("path2", "abc");

//     auto old = config->Set<const char*>("path", "world");
//     BOOST_CHECK(old);
//     BOOST_CHECK_EQUAL(old.value(), "hello");

//     BOOST_CHECK_EQUAL(config->Get<std::string>("path").value(), "world");
//     BOOST_CHECK_EQUAL(config->Get<std::string>("path2").value(), "abc");

//     auto v = config->Set<std::string>("path2", std::string("xyz"));
//     BOOST_CHECK(v);
//     BOOST_CHECK_EQUAL(v.value(), "abc");
//     BOOST_CHECK_EQUAL(config->Get<std::string>("path2").value(), "xyz");

//     auto sv_opt = config->Get<std::string>("path2");
//     BOOST_CHECK(sv_opt);
//     std::string_view sv(sv_opt->c_str());
//     BOOST_CHECK(sv == "xyz");
// }

// BOOST_AUTO_TEST_CASE(test_nonexistent_key_get_returns_nullopt) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("foo.bar", 100);
//     BOOST_CHECK(!config->Get<int>("foo.missing"));
//     BOOST_CHECK(!config->Get<std::string>("foo.nope"));
// }

// BOOST_AUTO_TEST_CASE(test_multiple_notify_change_callbacks) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("x", 10);

//     int first = 0;
//     int second = 0;
//     config->NotifyChange("x", [&](std::any new_value) {
//         first = std::any_cast<int>(new_value);
//     });
//     config->NotifyChange("x", [&](std::any new_value) {
//         second = std::any_cast<int>(new_value) + 1;
//     });

//     config->Set<int>("x", 42);
//     BOOST_CHECK_EQUAL(first, 42);
//     BOOST_CHECK_EQUAL(second, 43);
// }

// BOOST_AUTO_TEST_CASE(test_to_json_serialization_behavior) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("a", 1);
//     config->Register<bool>("b", false);
//     config->Register<std::string>("c", "hello");

//     config->Set<int>("a", 2);
//     config->Set<bool>("b", true);
//     config->Set<std::string>("c", "world");

//     std::string out = config->ToJson(false);
//     BOOST_CHECK(out.find("\"a\": 2") != std::string::npos);
//     BOOST_CHECK(out.find("\"b\": true") != std::string::npos);
//     BOOST_CHECK(out.find("\"c\": \"world\"") != std::string::npos);
// }

// BOOST_AUTO_TEST_CASE(test_from_json_invalid_string_throws) {
//     auto config = Config::Instance();
//     config->Reset();

//     BOOST_CHECK_THROW(config->FromJson("{invalid json}"), std::invalid_argument);
// }

// BOOST_AUTO_TEST_CASE(test_enum_to_json_from_json) {
//     auto config = Config::Instance();
//     config->Reset();

//     enum class Mode { OFF = 0, ON = 1, AUTO = 2 };
//     config->Register<Mode>("mode", Mode::AUTO);

//     config->Set<Mode>("mode", Mode::ON);
//     BOOST_CHECK_EQUAL(config->Get<Mode>("mode").value(), Mode::ON);

//     auto j = config->ToJson(false);
//     BOOST_CHECK(j.find("\"mode\"") != std::string::npos);

//     config->FromJson(R"({"mode": 0})");
//     BOOST_CHECK_EQUAL(config->Get<Mode>("mode").value(), Mode::OFF);
// }

// BOOST_AUTO_TEST_CASE(test_function_type_storage_and_call) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<std::function<int(int)>>("double", [](int x) {
//         return x * 2;
//     });
//     auto maybe_func = config->Get<std::function<int(int)>>("double");
//     BOOST_CHECK(maybe_func);
//     BOOST_CHECK_EQUAL(maybe_func.value()(3), 6);

//     config->Set<std::function<int(int)>>("double", [](int x) {
//         return x * 3;
//     });
//     BOOST_CHECK_EQUAL(config->Get<std::function<int(int)>>("double").value()(3), 9);
// }

// BOOST_AUTO_TEST_CASE(test_shared_ptr_lifecycle_in_config) {
//     auto config = Config::Instance();
//     config->Reset();

//     auto p = std::make_shared<int>(99);
//     config->Register<std::shared_ptr<int>>("ptr", p);

//     auto got = config->Get<std::shared_ptr<int>>("ptr");
//     BOOST_CHECK(got);
//     BOOST_CHECK_EQUAL(*got.value(), 99);

//     auto p2 = std::make_shared<int>(100);
//     config->Set<std::shared_ptr<int>>("ptr", p2);
//     BOOST_CHECK_EQUAL(*config->Get<std::shared_ptr<int>>("ptr").value(), 100);
// }

// BOOST_AUTO_TEST_CASE(test_set_old_value_for_serializable_types) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("k1", 5);
//     config->Register<std::string>("k2", "abc");

//     auto old1 = config->Set<int>("k1", 10);
//     BOOST_CHECK(old1);
//     BOOST_CHECK_EQUAL(old1.value(), 5);

//     auto old2 = config->Set<std::string>("k2", "def");
//     BOOST_CHECK(old2);
//     BOOST_CHECK_EQUAL(old2.value(), "abc");
// }

// BOOST_AUTO_TEST_CASE(test_set_old_value_for_non_serializable_returns_nullopt) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<std::shared_ptr<int>>("p", std::make_shared<int>(1));
//     auto oldp = config->Set<std::shared_ptr<int>>("p", std::make_shared<int>(2));
//     BOOST_CHECK(!oldp);
// }

// BOOST_AUTO_TEST_CASE(test_has_and_get_all_keys_nested_hierarchy) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("root.child1", 1);
//     config->Register<int>("root.child2", 2);
//     config->Register<int>("root.child2.grand", 3);

//     BOOST_CHECK(config->Has("root.child1"));
//     BOOST_CHECK(config->Has("root.child2.grand"));

//     auto keys = config->GetAllKeys();
//     BOOST_CHECK(std::find(keys.begin(), keys.end(), "root.child1") != keys.end());
//     BOOST_CHECK(std::find(keys.begin(), keys.end(), "root.child2.grand") != keys.end());
// }

// BOOST_AUTO_TEST_CASE(test_reset_restores_defaults_for_deep_nested) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("a.b.c", 10);
//     config->Register<std::string>("a.b.s", "hello");

//     config->Set<int>("a.b.c", 20);
//     config->Set<std::string>("a.b.s", "world");

//     config->Reset();
//     BOOST_CHECK_EQUAL(config->Get<int>("a.b.c").value(), 10);
//     BOOST_CHECK_EQUAL(config->Get<std::string>("a.b.s").value(), "hello");
// }

// BOOST_AUTO_TEST_CASE(test_type_mismatch_produces_runtime_error) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("ish", 11);
//     config->Set<int>("ish", 22);
//     BOOST_CHECK_THROW(config->Get<std::string>("ish"), std::runtime_error);
// }

// BOOST_AUTO_TEST_CASE(test_from_json_skips_non_serializable_subtrees) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<std::shared_ptr<int>>("nonser", std::make_shared<int>(7));
//     config->Register<int>("serial", 99);

//     config->FromJson(R"({"nonser": {"value": 100}, "serial": 123})");
//     BOOST_CHECK_EQUAL(config->Get<int>("serial").value(), 123);
//     BOOST_CHECK_EQUAL(*config->Get<std::shared_ptr<int>>("nonser").value(), 7);
// }

// BOOST_AUTO_TEST_CASE(test_dynamic_validator_closure) {
//     auto config = Config::Instance();
//     config->Reset();

//     int minv = 15;
//     config->Register<int>("dyn", [minv](bool is_set, int v) {
//         if (!is_set) return 20;
//         if (v < minv) return minv;
//         return v;
//     });

//     BOOST_CHECK_EQUAL(config->Get<int>("dyn").value(), 20);
//     config->Set<int>("dyn", 10);
//     BOOST_CHECK_EQUAL(config->Get<int>("dyn").value(), 15);
//     config->Set<int>("dyn", 17);
//     BOOST_CHECK_EQUAL(config->Get<int>("dyn").value(), 17);
// }

// BOOST_AUTO_TEST_CASE(test_bulk_register_get_performance_like_stress) {
//     auto config = Config::Instance();
//     config->Reset();

//     for (int i = 0; i < 100; ++i) {
//         config->Register<int>("bulk.key" + std::to_string(i), i);
//     }
//     for (int i = 0; i < 100; ++i) {
//         auto key = "bulk.key" + std::to_string(i);
//         BOOST_CHECK_EQUAL(config->Get<int>(key).value(), i);
//     }
// }

// // ==================== 以下为根据你提供的完整代码补全的新增测试 ====================

// BOOST_AUTO_TEST_CASE(test_once_flag_register_key) {
//     auto config = Config::Instance();
//     config->Reset();

//     std::once_flag flag;
//     std::call_once(flag, [&]() {
//         auto fps_validator = [](bool is_set, int v) {
//             if (!is_set) return 30;
//             if (v < 1) return 1;
//             if (v > 500) return 500;
//             return v;
//         };
//         auto codec_validator = [](bool is_set, const std::string& v) {
//             if (!is_set) return std::string("h264");
//             if (v == "h264" || v == "h265" || v == "vp9") return v;
//             return std::string("h264");
//         };
//         config->Register<const char*>("capture", nullptr);
//         config->Register<int>("capture.fps", fps_validator);
//         config->Register<std::string>("capture.codec", codec_validator);

//         for (int i = 0; i < 3; ++i) {
//             std::string prefix = "source_" + std::to_string(i);
//             config->Register<int>(prefix + ".fps", fps_validator);
//             config->Register<bool>(prefix + ".enable", true);
//             config->Register<std::string>(prefix + ".device", "/dev/video0");
//         }

//         int min_fps = 10;
//         auto dynamic_validator = [min_fps](bool is_set, int v) {
//             if (!is_set) return 30;
//             if (v < min_fps) return min_fps;
//             if (v > 200) return 200;
//             return v;
//         };
//         config->Register<int>("dynamic.fps", dynamic_validator);
//     });

//     config->Set("capture.fps", 800);
//     config->Set("source_0.fps", -10);
//     config->Set("source_1.fps", 60);
//     config->Set("capture.codec", std::string("av1"));

//     BOOST_CHECK_EQUAL(config->Get<int>("capture.fps").value(), 500);
//     BOOST_CHECK_EQUAL(config->Get<int>("source_0.fps").value(), 1);
//     BOOST_CHECK_EQUAL(config->Get<int>("source_1.fps").value(), 60);
//     BOOST_CHECK_EQUAL(config->Get<std::string>("capture.codec").value(), "h264");
//     BOOST_CHECK_EQUAL(config->Get<bool>("source_2.enable").value(), true);
// }

// BOOST_AUTO_TEST_CASE(test_optional_modify_not_affect_config) {
//     auto config = Config::Instance();
//     config->Reset();
//     config->Register<std::string>("capture.codec", "h264");

//     auto opt = config->Get<std::string>("capture.codec");
//     *opt = "modified";
//     BOOST_CHECK_EQUAL(config->Get<std::string>("capture.codec").value(), "h264");
// }

// BOOST_AUTO_TEST_CASE(test_shared_ptr_full_lifecycle_and_refcount) {
//     auto config = Config::Instance();
//     config->Reset();

//     auto obj_shared = std::make_shared<a_object>(20);
//     config->Register<std::shared_ptr<a_object>>("obj.shared", nullptr);
//     config->Set("obj.shared", obj_shared);

//     auto obj_opt = config->Get<std::shared_ptr<a_object>>("obj.shared");
//     BOOST_CHECK(obj_opt);
//     BOOST_CHECK_EQUAL(obj_opt->get()->value, 20);

//     BOOST_CHECK_EQUAL(obj_shared.use_count(), 3);
// }

// BOOST_AUTO_TEST_CASE(test_multiple_shared_objects_storage) {
//     auto config = Config::Instance();
//     config->Reset();

//     auto obj1 = std::make_shared<a_object>(100);
//     auto obj2 = std::make_shared<a_object>(200);
//     auto obj3 = std::make_shared<a_object>(300);

//     config->Register<std::shared_ptr<a_object>>("objects.obj1", nullptr);
//     config->Register<std::shared_ptr<a_object>>("objects.obj2", nullptr);
//     config->Register<std::shared_ptr<a_object>>("objects.obj3", nullptr);

//     config->Set("objects.obj1", obj1);
//     config->Set("objects.obj2", obj2);
//     config->Set("objects.obj3", obj3);

//     auto opt1 = config->Get<std::shared_ptr<a_object>>("objects.obj1");
//     auto opt2 = config->Get<std::shared_ptr<a_object>>("objects.obj2");
//     auto opt3 = config->Get<std::shared_ptr<a_object>>("objects.obj3");

//     BOOST_CHECK(opt1 && opt1->get()->value == 100);
//     BOOST_CHECK(opt2 && opt2->get()->value == 200);
//     BOOST_CHECK(opt3 && opt3->get()->value == 300);
// }

// BOOST_AUTO_TEST_CASE(test_function_and_object_mixed_storage) {
//     auto config = Config::Instance();
//     config->Reset();

//     auto obj = std::make_shared<a_object>(50);
//     config->Register<std::shared_ptr<a_object>>("mixed.obj", nullptr);
//     config->Set("mixed.obj", obj);

//     config->Register<std::function<int(int)>>("mixed.func", [](int x) {
//         return x * 3;
//     });

//     auto obj_opt = config->Get<std::shared_ptr<a_object>>("mixed.obj");
//     BOOST_CHECK(obj_opt && obj_opt->get()->value == 50);

//     auto func_opt = config->Get<std::function<int(int)>>("mixed.func");
//     BOOST_CHECK(func_opt);
//     BOOST_CHECK_EQUAL((*func_opt)(5), 15);
// }

// BOOST_AUTO_TEST_CASE(test_json_skip_all_non_serializable_types) {
//     auto config = Config::Instance();
//     config->Reset();

//     config->Register<int>("data.count", 0);
//     config->Register<std::string>("data.name", "test");
//     config->Register<std::shared_ptr<a_object>>("data.obj", nullptr);
//     config->Register<std::function<void()>>("data.callback", nullptr);

//     config->Set("data.count", 42);
//     config->Set("data.name", "hello");
//     config->Set("data.obj", std::make_shared<a_object>(99));
//     config->Set("data.callback", []() {
//     });

//     std::string json = config->ToJson(true);
//     BOOST_CHECK(json.find("count") != std::string::npos);
//     BOOST_CHECK(json.find("42") != std::string::npos);
//     BOOST_CHECK(json.find("hello") != std::string::npos);
//     BOOST_CHECK(json.find("__non_serializable") != std::string::npos);
// }

// BOOST_AUTO_TEST_CASE(test_smart_ptr_default_value) {
//     auto config = Config::Instance();
//     config->Reset();

//     auto default_obj = std::make_shared<a_object>(777);
//     config->Register<std::shared_ptr<a_object>>("default.obj", default_obj);

//     auto opt = config->Get<std::shared_ptr<a_object>>("default.obj");
//     BOOST_CHECK(opt);
//     BOOST_CHECK_EQUAL(opt->get()->value, 777);
// }

// BOOST_AUTO_TEST_CASE(test_object_modification_through_shared_ptr) {
//     auto config = Config::Instance();
//     config->Reset();

//     auto obj = std::make_shared<a_object>(11);
//     config->Register<std::shared_ptr<a_object>>("modify.obj", nullptr);
//     config->Set("modify.obj", obj);

//     obj->value = 22;
//     auto opt = config->Get<std::shared_ptr<a_object>>("modify.obj");
//     BOOST_CHECK(opt && opt->get()->value == 22);

//     auto opt2 = config->Get<std::shared_ptr<a_object>>("modify.obj");
//     opt2->get()->value = 33;
//     auto opt3 = config->Get<std::shared_ptr<a_object>>("modify.obj");
//     BOOST_CHECK(opt3 && opt3->get()->value == 33);
// }

// BOOST_AUTO_TEST_CASE(test_parse_config_full_json) {
//     auto config = Config::Instance();
//     config->Reset();

//     std::once_flag flag;
//     std::call_once(flag, [&]() {
//         auto fps_validator = [](bool is_set, int v) {
//             if (!is_set) return 30;
//             return std::clamp(v, 1, 500);
//         };
//         config->Register<const char*>("capture", nullptr);
//         config->Register<int>("capture.fps", fps_validator);
//         config->Register<std::string>("capture.codec", [](bool, const std::string& v) {
//             return (v == "h264" || v == "h265" || v == "vp9") ? v : "h264";
//         });
//         for (int i = 0; i < 3; ++i) {
//             std::string p = "source_" + std::to_string(i);
//             config->Register<int>(p + ".fps", fps_validator);
//             config->Register<bool>(p + ".enable", true);
//             config->Register<std::string>(p + ".device", "/dev/video0");
//         }
//         config->Register<int>("dynamic.fps", [](bool, int v) {
//             return v;
//         });
//     });

//     std::string json_str = R"({
//   "capture": {
//     "__value": "hello char",
//     "codec": "h264",
//     "fps": 100
//   },
//   "dynamic": {
//     "fps": 10
//   },
//   "source_0": {
//     "device": "/dev/video0",
//     "enable": true,
//     "fps": 1
//   },
//   "source_1": {
//     "device": "/dev/video0",
//     "enable": true,
//     "fps": 60
//   },
//   "source_2": {
//     "device": "/dev/video0",
//     "enable": true,
//     "fps": 30
//   }
// })";

//     config->FromJson(json_str);
//     BOOST_CHECK_EQUAL(config->Get<std::string>("capture").value(), "hello char");
//     BOOST_CHECK_EQUAL(config->Get<int>("capture.fps").value(), 100);
//     BOOST_CHECK_EQUAL(config->Get<int>("dynamic.fps").value(), 10);
//     BOOST_CHECK_EQUAL(config->Get<std::string>("source_0.device").value(), "/dev/video0");
// }

BOOST_AUTO_TEST_SUITE_END()