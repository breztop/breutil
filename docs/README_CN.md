
# breutil

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![CMake](https://img.shields.io/badge/CMake-3.20+-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

breutil 是一个现代 C++ 工具库集合，旨在为 C++ 开发者提供基础组件框架，用于便捷开发。
当项目逐渐变大时，可以使用市场上更成熟的库，如 Boost等

## 特性

- **仅头文件设计**：所有类工具无需编译，直接包含即可使用（依赖其他库的封装库除外）
- **跨平台支持**：除了部分`系统/工具`封装相关模块，仅依赖`std`
- **现代 C++**：使用 C++23/20 标准特性
- **模块化组织**：按功能划分模块，便于按需使用
- **测试驱动**：提供完整的测试套件，确保可靠性

## 依赖

当然你完全可以不用下载，除非你使用，建议下载asio，也是一个仅头文件库
部分模块需要第三方库：
- **OpenSSL**：用于加密模块 (`crypto`)（可选）
- **MySQL Connector/C++**：用于数据库模块 (`database`)（可选）
- **hiredis**：用于 Redis 连接 (`database`)（可选）
- **Asio**：用于网络模块 (`net`)（可选）
- **spdlog**：用于日志模块 (`spdlog`)（可选）
- **Zlib**：用于压缩模块 (`zlib`)（可选）
- **Boost.Test**：用于测试套件（可选）


### 集成到您的项目

先安装 breutil：

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/your/install/prefix
cmake --build build
cmake --install build
# 卸载
cmake --build build --target uninstall
```

在您的 `CMakeLists.txt` 中添加：

```cmake
set(breutil_DIR "/your/install/prefix/lib/cmake/breutil")
find_package(breutil CONFIG REQUIRED)

target_link_libraries(your_target PRIVATE breutil::breutil)
```

## 模块说明

| 模块名称 | 类型 | 描述 |
| -------- | ---- | ---- |
| `algorithm` | 算法 | 常用算法实现，如移动平均、中缀表达式求值、前缀树 |
| `core` | 核心工具 | 基础工具，如最大最小值计算、范围操作 |
| `crypto` | 加密 | 基于 OpenSSL 的加密函数 (AES, RSA, ECC, ECDSA) |
| `data_struct` | 数据结构 | 额外数据结构实现，如二叉树 |
| `database` | 数据库 | MySQL 和 Redis 连接池封装 |
| `encoding` | 编码转换 | Base64、十六进制转换等 |
| `flag` | 命令行参数 | 仿 Golang flag 的参数解析工具 |
| `hash` | 哈希函数 | CRC32、MD5、SHA1 哈希算法 |
| `ini` | 配置加载 | INI 配置文件解析和加载 |
| `json` | JSON 处理 | 轻量级仅头文件 JSON 解析库 |
| `math` | 数学库 | 数学计算工具 (开发中) |
| `mouse_key_hook` | 系统钩子 | 鼠标键盘事件钩子 |
| `net` | 网络 | 基于 Asio 的高级网络功能 |
| `signal` | 信号处理 | 信号处理工具 (开发中) |
| `string` | 字符串增强 | std::string 扩展功能 (开发中) |
| `sys_*` | 系统工具 | 平台特定工具 (Linux/macOS/Windows) |
| `uuid` | UUID 生成 | 跨平台 UUID 生成 |

## 核心工具类

| 工具类 | 类别 | 描述 |
| ------ | ---- | ---- |
| `Buffer` | 数据处理 | 减少数据复制的缓冲区队列 |
| `BlockQueue` | 并发 | 多线程安全队列 |
| `EasyTest` | 测试 | 简单易用的测试框架 |
| `NifixExpression` | 算法 | 中缀表达式求值 (+-*/) |
| `Log` | 日志 | 日志记录工具 |
| `BlockQueue` | 并发 | 阻塞队列 |
| `Date` | 时间 | 日期处理工具 |
| `Defer` | 工具 | Go 风格的 defer 机制 |
| `Enum` | 工具 | 枚举增强工具 |
| `EventBus` | 事件 | 事件总线实现 |
| `FileDir` | 文件系统 | 文件和目录操作 |
| `FSM` | 状态机 | 有限状态机 |
| `HexLook` | 调试 | 十六进制查看器 |
| `LibExport` | 工具 | 库导出宏 |
| `ObjectPool` | 内存 | 对象池管理 |
| `OstreamOperator` | I/O | 输出流操作符重载 |
| `Platform` | 系统 | 平台检测工具 |
| `Property` | 配置 | 属性管理 |
| `RingBuffer` | 数据结构 | 环形缓冲区 |
| `Signal` | 信号 | 信号处理 |
| `Singleton` | 设计模式 | 单例模式实现 |
| `Spdlog` | 日志 | spdlog 封装 |
| `StringFunc` | 字符串 | 字符串处理函数 |
| `System` | 系统 | 系统信息获取 |
| `ThreadPool` | 并发 | 线程池 |
| `Time` | 时间 | 时间处理工具 |
| `Timer` | 时间 | 定时器 |
| `TypeError` | 错误处理 | 类型错误处理 |
| `UsbListener` | 硬件 | USB 设备监听 |
| `Zlib` | 压缩 | Zlib 压缩工具 |

## 使用示例

### JSON 解析

```cpp
#include "breutil/json/json_value.hpp"
#include "breutil/json/json_parse.hpp"

using namespace bre::json;

// 解析 JSON 字符串
std::string json_str = R"(
{
    "name": "breutil",
    "version": "1.0",
    "features": ["header-only", "cross-platform", "modern-cpp"]
}
)";

try {
    Value root = parse(json_str);
    std::string name = root["name"].asString();
    int version = root["version"].asInt();
    // 使用数据...
} catch (const ParseError& e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
}
```

### 简单测试

```cpp
#include "breutil/easy_test.hpp"

TEST_CASE("Basic assertions") {
    ASSERT_TRUE(1 + 1 == 2);
    ASSERT_EQ(42, 42);
    ASSERT_STR_EQ("hello", "hello");
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}
```

### 命令行参数解析

```cpp
#include "breutil/flag/flag.hpp"

int main(int argc, char* argv[]) {
    bre::Flag flag;
    std::string name = flag.String("name", "world", "Name to greet");
    int count = flag.Int("count", 1, "Number of greetings");

    flag.Parse(argc, argv);

    for (int i = 0; i < count; ++i) {
        std::cout << "Hello, " << name << "!" << std::endl;
    }

    return 0;
}
```

## 测试

项目包含完整的测试套件。测试文件命名规则：

- `test_xxx.hpp` 或 `test_xxx.cpp`：对应 `xxx` 模块的测试
- 测试文件位于 `test/` 目录、同级目录或对应模块目录下

运行测试：

```bash
cd build
ctest --output-on-failure
```

## 贡献

欢迎贡献任何代码！请遵循以下步骤：

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

## 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 联系

项目维护者：breztop

项目链接：[https://github.com/breztop/breutil](https://github.com/breztop/breutil)

---

**注意**：test_xxx 开头的文件是 xxx 文件的测试。为了方便使用，breutil 中的工具尽可能写为仅头文件，部分会依赖第三方库。一个类的测试文件一般在 test 目录下、同级目录下，或该文件对应的文件夹下。对于功能较多的文件，功能测试放到了 signal 中。

