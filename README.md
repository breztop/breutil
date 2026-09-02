# breutil

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![CMake](https://img.shields.io/badge/CMake-3.20%2B-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

**English** | [简体中文](docs/README_CN.md)

`breutil` is a collection of modern C++ utility libraries designed to provide lightweight, reusable building blocks for everyday C++ development.

The project focuses on keeping common utilities simple, modular, and easy to integrate. Most components are header-only and depend only on the C++ standard library.

For larger or more complex projects, mature libraries such as [Boost](https://www.boost.org/) may be a better choice for some functionality.

## Features

* **Header-only design**
  Most utilities require no separate compilation and can be used by simply including the corresponding headers. Modules that wrap third-party libraries are exceptions.

* **Cross-platform support**
  Most modules depend only on the C++ standard library. Platform-specific functionality is isolated in system-related modules.

* **Modern C++**
  Designed around C++20, with selected C++23 features used where appropriate.

* **Modular organization**
  Components are grouped by functionality so you can include only what your project needs.

* **Testing support**
  The project includes tests for its major modules and utilities.

## Dependencies

Most of `breutil` can be used without installing any third-party libraries.

Optional modules may require external dependencies:

| Dependency          | Used By    | Required |
| ------------------- | ---------- | -------- |
| OpenSSL             | `crypto`   | Optional |
| MySQL Connector/C++ | `database` | Optional |
| hiredis             | `database` | Optional |
| Asio                | `net`      | Optional |
| spdlog              | `spdlog`   | Optional |
| Zlib                | `zlib`     | Optional |
| Boost.Test          | Test suite | Optional |

If you plan to use the networking module, installing standalone Asio is recommended. Asio itself can also be used as a header-only library.

## Installation

Install `breutil` with CMake:

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/your/install/prefix
cmake --build build
cmake --install build
```

To uninstall:

```bash
cmake --build build --target uninstall
```

Then add `breutil` to your project's `CMakeLists.txt`:

```cmake
set(breutil_DIR "/your/install/prefix/lib/cmake/breutil")

find_package(breutil CONFIG REQUIRED)

target_link_libraries(your_target PRIVATE breutil::breutil)
```

## Modules

| Module           | Category               | Description                                                                        |
| ---------------- | ---------------------- | ---------------------------------------------------------------------------------- |
| `algorithm`      | Algorithms             | Common algorithms such as moving averages, infix expression evaluation, and tries  |
| `core`           | Core utilities         | Basic reusable utilities, range operations, min/max helpers, and common components |
| `crypto`         | Cryptography           | OpenSSL-based cryptographic utilities including AES, RSA, ECC, and ECDSA           |
| `data_struct`    | Data structures        | Additional data structures such as binary trees                                    |
| `database`       | Database               | MySQL and Redis connection pool wrappers                                           |
| `encoding`       | Encoding               | Base64, hexadecimal conversion, and related encoding utilities                     |
| `flag`           | Command-line arguments | Go-style command-line flag parsing                                                 |
| `hash`           | Hashing                | CRC32, MD5, SHA-1, and related hash functions                                      |
| `ini`            | Configuration          | INI configuration file parsing and loading                                         |
| `json`           | JSON                   | Lightweight header-only JSON parser                                                |
| `math`           | Mathematics            | Mathematical utilities *(under development)*                                       |
| `mouse_key_hook` | System hooks           | Mouse and keyboard event hooks                                                     |
| `net`            | Networking             | Higher-level networking utilities built on Asio                                    |
| `signal`         | Signal handling        | Signal-related utilities *(under development)*                                     |
| `string`         | String utilities       | Extensions and helpers for `std::string` *(under development)*                     |
| `sys_*`          | System utilities       | Platform-specific utilities for Linux, macOS, and Windows                          |
| `uuid`           | UUID                   | Cross-platform UUID generation                                                     |

## Core Utilities

| Utility           | Category        | Description                                                                    |
| ----------------- | --------------- | ------------------------------------------------------------------------------ |
| `Buffer`          | Data processing | Buffer queue designed to reduce unnecessary data copies                        |
| `BlockQueue`      | Concurrency     | Thread-safe blocking queue                                                     |
| `EasyTest`        | Testing         | Lightweight testing framework                                                  |
| `NifixExpression` | Algorithm       | Infix expression evaluator supporting operators such as `+`, `-`, `*`, and `/` |
| `Log`             | Logging         | Logging utility                                                                |
| `Date`            | Time            | Date handling utilities                                                        |
| `Defer`           | Utility         | Go-style `defer` mechanism                                                     |
| `Enum`            | Utility         | Enum helper utilities                                                          |
| `EventBus`        | Events          | Event bus implementation                                                       |
| `FileDir`         | Filesystem      | File and directory operations                                                  |
| `FSM`             | State machine   | Finite-state machine implementation                                            |
| `HexLook`         | Debugging       | Hexadecimal data viewer                                                        |
| `LibExport`       | Utility         | Shared-library export macros                                                   |
| `ObjectPool`      | Memory          | Object pool implementation                                                     |
| `OstreamOperator` | I/O             | Helpers for output stream operators                                            |
| `Platform`        | System          | Platform detection utilities                                                   |
| `Property`        | Configuration   | Property management utility                                                    |
| `RingBuffer`      | Data structures | Ring buffer implementation                                                     |
| `Signal`          | Signals         | Signal-related utility                                                         |
| `Singleton`       | Design patterns | Singleton implementation                                                       |
| `Spdlog`          | Logging         | Wrapper around spdlog                                                          |
| `StringFunc`      | Strings         | String manipulation functions                                                  |
| `System`          | System          | System information utilities                                                   |
| `ThreadPool`      | Concurrency     | Thread pool implementation                                                     |
| `Time`            | Time            | Time-related utilities                                                         |
| `Timer`           | Time            | Timer utility                                                                  |
| `TypeError`       | Error handling  | Type-related error handling                                                    |
| `UsbListener`     | Hardware        | USB device monitoring                                                          |
| `Zlib`            | Compression     | Zlib compression utilities                                                     |

## Usage Examples

### JSON Parsing

```cpp
#include "breutil/json/json_value.hpp"
#include "breutil/json/json_parse.hpp"

#include <iostream>
#include <string>

using namespace bre::json;

int main() {
    std::string json_str = R"(
    {
        "name": "breutil",
        "version": 1,
        "features": [
            "header-only",
            "cross-platform",
            "modern-cpp"
        ]
    }
    )";

    try {
        Value root = parse(json_str);

        std::string name = root["name"].asString();
        int version = root["version"].asInt();

        std::cout << "name: " << name << '\n';
        std::cout << "version: " << version << '\n';
    } catch (const ParseError& e) {
        std::cerr << "Parse error: " << e.what() << '\n';
    }
}
```

### Simple Testing

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

### Command-Line Argument Parsing

```cpp
#include "breutil/flag/flag.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    bre::Flag flag;

    std::string name =
        flag.String("name", "world", "Name to greet");

    int count =
        flag.Int("count", 1, "Number of greetings");

    flag.Parse(argc, argv);

    for (int i = 0; i < count; ++i) {
        std::cout << "Hello, " << name << "!\n";
    }

    return 0;
}
```

## Testing

The project includes tests for its modules and utilities.

Test files generally follow these naming conventions:

```text
test_xxx.hpp
test_xxx.cpp
```

where `xxx` corresponds to the component or module being tested.

Tests may be located in:

```text
test/
```

or alongside the corresponding source/header file or module directory.

Run the test suite with:

```bash
cmake -S . -B build
cmake --build build

cd build
ctest --output-on-failure
```

Some components with more extensive functionality may have additional functional tests organized separately.

## Project Structure

A typical `breutil` source tree looks like:

```text
breutil/
├── algorithm/
├── core/
├── crypto/
├── data_struct/
├── database/
├── encoding/
├── flag/
├── hash/
├── ini/
├── json/
├── math/
├── net/
├── signal/
├── string/
├── sys_linux/
├── sys_macos/
├── sys_windows/
└── uuid/
```

Individual modules can generally be used independently.

For most header-only utilities, simply include the required header:

```cpp
#include "breutil/..."
```

Third-party dependencies are only required when using modules that explicitly depend on them.

## Design Philosophy

`breutil` is intended to provide small and practical utilities for C++ projects without requiring a large framework.

The main goals are:

* Keep common utilities lightweight.
* Prefer header-only implementations when practical.
* Minimize unnecessary third-party dependencies.
* Keep platform-specific code isolated.
* Provide reusable building blocks instead of large abstractions.
* Make individual modules easy to integrate into existing projects.
* Use modern C++ features while keeping the APIs straightforward.

`breutil` is not intended to replace mature general-purpose libraries such as Boost. As a project grows, using established libraries for complex functionality is often the better engineering choice.

## Contributing

Contributions are welcome.

To contribute:

1. Fork the repository.

2. Create a feature branch:

```bash
git checkout -b feature/AmazingFeature
```

3. Commit your changes:

```bash
git commit -m "Add some AmazingFeature"
```

4. Push the branch:

```bash
git push origin feature/AmazingFeature
```

5. Open a Pull Request.

When adding new utilities, tests should normally be added alongside the implementation or in the corresponding test directory.

## License

This project is licensed under the MIT License.

See [LICENSE](LICENSE) for details.

## Maintainer

Maintainer: **breztop**

Project repository:

https://github.com/breztop/breutil

---

> **Note**
>
> Files beginning with `test_` are generally tests for the corresponding component.
>
> To keep integration simple, utilities in `breutil` are implemented as header-only components whenever practical. Some modules require optional third-party libraries.
>
> Tests are usually located in the `test/` directory, alongside the corresponding source file, or inside the related module directory.
