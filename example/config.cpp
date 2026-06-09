#define BRE_CONFIG_JSON_SUPPORT 1
#include "breutil/config.hpp"

#include <iostream>
#include <memory>
#include <string>

using namespace bre;

int main() {
    auto config = Config::Instance();
    config->Reset();

    config->Register<int>("app.fps", 30);
    config->Register<std::string>("app.codec", "h264");
    config->Register<bool>("app.audio", true);

    auto fps = config->Get<int>("app.fps");
    if (fps) {
        std::cout << "FPS: " << fps.value() << std::endl;
    }

    auto codec = config->Get<std::string>("app.codec");
    if (codec) {
        std::cout << "Codec: " << codec.value() << std::endl;
    }

    config->Set<int>("app.fps", 60);
    std::cout << "Updated FPS: " << config->Get<int>("app.fps").value() << std::endl;

    auto old_fps = config->Set<int>("app.fps", 120);
    if (old_fps) {
        std::cout << "Old FPS was: " << old_fps.value() << std::endl;
    }

    config->Register<int>("capture.width", [](bool is_set, int v) {
        if (!is_set) return 1920;
        return v > 0 ? v : 1920;
    });
    std::cout << "Default width: " << config->Get<int>("capture.width").value() << std::endl;

    config->Set<int>("capture.width", 3840);
    std::cout << "Custom width: " << config->Get<int>("capture.width").value() << std::endl;

    config->Set<int>("capture.width", -100);
    std::cout << "Clamped width: " << config->Get<int>("capture.width").value() << std::endl;

    bool called = false;
    int new_value = 0;
    config->NotifyChange("app.fps", [&](std::any val) {
        called = true;
        new_value = std::any_cast<int>(val);
    });

    config->Set<int>("app.fps", 90);
    std::cout << "Callback fired: " << (called ? "yes" : "no") << ", value: " << new_value
              << std::endl;

    auto keys = config->GetAllKeys();
    std::cout << "All keys: ";
    for (const auto& k : keys) {
        std::cout << k << " ";
    }
    std::cout << std::endl;

    std::cout << "Has app.codec: " << (config->Has("app.codec") ? "yes" : "no") << std::endl;
    std::cout << "Has missing.key: " << (config->Has("missing.key") ? "yes" : "no") << std::endl;

    std::string json = config->ToJson(true);
    std::cout << "JSON output:\n" << json << std::endl;

    std::string new_json = R"({
      "app": {
        "fps": 240,
        "codec": "h265",
        "audio": false
      }
    })";
    config->FromJson(new_json);
    std::cout << "After loading JSON - FPS: " << config->Get<int>("app.fps").value() << std::endl;
    std::cout << "After loading JSON - Codec: " << config->Get<std::string>("app.codec").value()
              << std::endl;

    config->Reset();
    std::cout << "After reset - FPS: " << config->Get<int>("app.fps").value() << std::endl;
    std::cout << "After reset - Codec: " << config->Get<std::string>("app.codec").value()
              << std::endl;

    return 0;
}
