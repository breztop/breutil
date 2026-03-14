#pragma once
#include <cassert>
#include <fstream>
#include <iostream>

#include "ini.hpp"

void testIniExample() {
    std::cout << "\n=== Testing INI Example File ===" << std::endl;

    bre::Ini& config = bre::Ini::Instance("ini_example.ini");

    // 测试 Database 配置段
    std::string host = config.GetStr("Database", "Host", "");
    int port = config.GetInt("Database", "Port", 0);
    std::string username = config.GetStr("Database", "Username", "");
    std::string password = config.GetStr("Database", "Password", "");
    std::string database = config.GetStr("Database", "Database", "");
    int maxConnections = config.GetInt("Database", "MaxConnections", 0);
    double timeout = config.GetDouble("Database", "Timeout", 0.0);
    bool enableSSL = config.GetBool("Database", "EnableSSL", false);
    bool autoReconnect = config.GetBool("Database", "AutoReconnect", false);

    assert(host == "localhost");
    assert(port == 3306);
    assert(username == "admin");
    assert(password == "my_secure_password");
    assert(database == "mydb");
    assert(maxConnections == 100);
    assert(std::abs(timeout - 30.5) < 0.01);
    assert(enableSSL == true);
    assert(autoReconnect == true);
    std::cout << "✓ Database section tests passed" << std::endl;

    // 测试 Server 配置段
    std::string listenAddr = config.GetStr("Server", "ListenAddress", "");
    int listenPort = config.GetInt("Server", "ListenPort", 0);
    int maxThreads = config.GetInt("Server", "MaxThreads", 0);
    bool debug = config.GetBool("Server", "Debug", true);
    std::string logLevel = config.GetStr("Server", "LogLevel", "");
    std::string rootPath = config.GetStr("Server", "RootPath", "");

    assert(listenAddr == "0.0.0.0");
    assert(listenPort == 5432);
    assert(maxThreads == 16);
    assert(debug == false);
    assert(logLevel == "INFO");
    assert(rootPath == "/var/www/html");
    std::cout << "✓ Server section tests passed" << std::endl;

    // 测试 Application 配置段
    std::string appName = config.GetStr("Application", "Name", "");
    std::string version = config.GetStr("Application", "Version", "");
    std::string author = config.GetStr("Application", "Author", "");
    bool featureX = config.GetBool("Application", "EnableFeatureX", false);
    bool featureY = config.GetBool("Application", "EnableFeatureY", true);

    assert(appName == "My Application");
    assert(version == "1.2.3");
    assert(author == "John Doe");
    assert(featureX == true);
    assert(featureY == false);
    std::cout << "✓ Application section tests passed" << std::endl;

    // 测试 Cache 配置段
    bool cacheEnabled = config.GetBool("Cache", "Enabled", false);
    std::string cacheType = config.GetStr("Cache", "Type", "");
    std::string cacheHost = config.GetStr("Cache", "Host", "");
    int cachePort = config.GetInt("Cache", "Port", 0);
    int ttl = config.GetInt("Cache", "TTL", 0);
    int maxMemory = config.GetInt("Cache", "MaxMemory", 0);

    assert(cacheEnabled == true);
    assert(cacheType == "redis");
    assert(cacheHost == "127.0.0.1");
    assert(cachePort == 6379);
    assert(ttl == 3600);
    assert(maxMemory == 512);
    std::cout << "✓ Cache section tests passed" << std::endl;

    // 测试 Defaults 配置段（空值和特殊值）
    std::string emptyValue = config.GetStr("Defaults", "EmptyValue", "default");
    std::string nullValue = config.GetStr("Defaults", "NullValue", "default");
    int zeroInt = config.GetInt("Defaults", "ZeroInt", -1);
    double zeroFloat = config.GetDouble("Defaults", "ZeroFloat", -1.0);
    bool falseValue = config.GetBool("Defaults", "FalseValue", true);

    assert(emptyValue == "" || emptyValue == "default");
    assert(nullValue == "");
    assert(zeroInt == 0);
    assert(std::abs(zeroFloat - 0.0) < 0.001);
    assert(falseValue == false);
    std::cout << "✓ Defaults section tests passed" << std::endl;
}


// 测试类型转换功能
void testTypeConversion() {
    std::cout << "\n=== Testing Type Conversion ===" << std::endl;

    // 创建测试配置文件
    std::ofstream testFile("test_types.ini");
    testFile << "[Numbers]\n";
    testFile << "IntValue=42\n";
    testFile << "DoubleValue=3.14159\n";
    testFile << "NegativeInt=-100\n";
    testFile << "NegativeDouble=-2.718\n";
    testFile << "\n[Boolean]\n";
    testFile << "BoolTrue1=true\n";
    testFile << "BoolTrue2=TRUE\n";
    testFile << "BoolTrue3=1\n";
    testFile << "BoolTrue4=yes\n";
    testFile << "BoolTrue5=on\n";
    testFile << "BoolFalse1=false\n";
    testFile << "BoolFalse2=FALSE\n";
    testFile << "BoolFalse3=0\n";
    testFile << "BoolFalse4=no\n";
    testFile << "BoolFalse5=off\n";
    testFile << "\n[Invalid]\n";
    testFile << "InvalidInt=abc\n";
    testFile << "InvalidDouble=xyz\n";
    testFile.close();

    bre::Ini& config = bre::Ini::Instance("test_types.ini");

    // 测试整数
    assert(config.GetInt("Numbers", "IntValue") == 42);
    std::cout << "✓ Integer conversion works" << std::endl;

    assert(config.GetInt("Numbers", "NegativeInt") == -100);
    std::cout << "✓ Negative integer conversion works" << std::endl;

    // 测试浮点数
    assert(std::abs(config.GetDouble("Numbers", "DoubleValue") - 3.14159) < 0.00001);
    std::cout << "✓ Double conversion works" << std::endl;

    assert(std::abs(config.GetDouble("Numbers", "NegativeDouble") - (-2.718)) < 0.001);
    std::cout << "✓ Negative double conversion works" << std::endl;

    // 测试布尔值
    assert(config.GetBool("Boolean", "BoolTrue1") == true);
    assert(config.GetBool("Boolean", "BoolTrue2") == true);
    assert(config.GetBool("Boolean", "BoolTrue3") == true);
    assert(config.GetBool("Boolean", "BoolTrue4") == true);
    assert(config.GetBool("Boolean", "BoolTrue5") == true);
    std::cout << "✓ Boolean true values work" << std::endl;

    assert(config.GetBool("Boolean", "BoolFalse1") == false);
    assert(config.GetBool("Boolean", "BoolFalse2") == false);
    assert(config.GetBool("Boolean", "BoolFalse3") == false);
    assert(config.GetBool("Boolean", "BoolFalse4") == false);
    assert(config.GetBool("Boolean", "BoolFalse5") == false);
    std::cout << "✓ Boolean false values work" << std::endl;

    // 测试无效值的默认值处理
    assert(config.GetInt("Invalid", "InvalidInt", 999) == 999);
    assert(config.GetDouble("Invalid", "InvalidDouble", 1.23) == 1.23);
    std::cout << "✓ Default values for invalid conversions work" << std::endl;
}

// 测试注释和特殊字符
void testCommentsAndSpecialChars() {
    std::cout << "\n=== Testing Comments and Special Characters ===" << std::endl;

    std::ofstream testFile("test_special.ini");
    testFile << "; This is a comment with semicolon\n";
    testFile << "# This is a comment with hash\n";
    testFile << "[Section1]\n";
    testFile << "Key1=Value1 ; inline comment\n";
    testFile << "Key2=Value2 # another inline comment\n";
    testFile << "QuotedString=\"Hello World\"\n";
    testFile << "SingleQuoted='Single Quote String'\n";
    testFile << "EscapedChars=\"Line1\\nLine2\\tTabbed\"\n";
    testFile << "PathWithBackslash=\"C:\\\\Users\\\\Test\"\n";
    testFile << "EmptyQuotes=\"\"\n";
    testFile.close();

    bre::Ini& config = bre::Ini::Instance("test_special.ini");

    assert(config.GetStr("Section1", "Key1") == "Value1");
    std::cout << "✓ Semicolon comments handled correctly" << std::endl;

    assert(config.GetStr("Section1", "Key2") == "Value2");
    std::cout << "✓ Hash comments handled correctly" << std::endl;

    assert(config.GetStr("Section1", "QuotedString") == "Hello World");
    std::cout << "✓ Double quoted strings work" << std::endl;

    assert(config.GetStr("Section1", "SingleQuoted") == "Single Quote String");
    std::cout << "✓ Single quoted strings work" << std::endl;

    std::string escaped = config.GetStr("Section1", "EscapedChars");
    assert(escaped.find('\n') != std::string::npos);
    assert(escaped.find('\t') != std::string::npos);
    std::cout << "✓ Escape characters work" << std::endl;

    assert(config.GetStr("Section1", "PathWithBackslash") == "C:\\Users\\Test");
    std::cout << "✓ Backslash escaping works" << std::endl;

    assert(config.GetStr("Section1", "EmptyQuotes") == "");
    std::cout << "✓ Empty quoted strings work" << std::endl;
}

// 测试修改和保存功能
void testModifyAndSave() {
    std::cout << "\n=== Testing Modify and Save ===" << std::endl;

    bre::Ini& config = bre::Ini::Instance("test_modify.ini", true);

    // 设置各种类型的值
    config.Set("NewSection", "StringValue", "TestString");
    config.Set("NewSection", "IntValue", 123);
    config.Set("NewSection", "DoubleValue", 45.67);
    config.Set("NewSection", "BoolValue", true);

    // 验证设置的值
    assert(config.GetStr("NewSection", "StringValue") == "TestString");
    assert(config.GetInt("NewSection", "IntValue") == 123);
    assert(std::abs(config.GetDouble("NewSection", "DoubleValue") - 45.67) < 0.01);
    assert(config.GetBool("NewSection", "BoolValue") == true);
    std::cout << "✓ Setting values works" << std::endl;

    // 保存到文件
    bool saved = config.Save();
    assert(saved == true);
    std::cout << "✓ Saving to file works" << std::endl;

    // 重新加载验证
    bre::Ini& config2 = bre::Ini::Instance("test_modify.ini");
    assert(config2.GetStr("NewSection", "StringValue") == "TestString");
    assert(config2.GetInt("NewSection", "IntValue") == 123);
    std::cout << "✓ Reloading saved file works" << std::endl;
}

// 测试查询功能
void testQueryFunctions() {
    std::cout << "\n=== Testing Query Functions ===" << std::endl;

    std::ofstream testFile("test_query.ini");
    testFile << "[Section1]\n";
    testFile << "Key1=Value1\n";
    testFile << "Key2=Value2\n";
    testFile << "\n[Section2]\n";
    testFile << "KeyA=ValueA\n";
    testFile.close();

    bre::Ini& config = bre::Ini::Instance("test_query.ini");

    // 测试HasSection
    assert(config.HasSection("Section1") == true);
    assert(config.HasSection("Section2") == true);
    assert(config.HasSection("NonExistent") == false);
    std::cout << "✓ HasSection works" << std::endl;

    // 测试HasKey
    assert(config.HasKey("Section1", "Key1") == true);
    assert(config.HasKey("Section1", "Key2") == true);
    assert(config.HasKey("Section1", "NonExistent") == false);
    assert(config.HasKey("NonExistent", "Key1") == false);
    std::cout << "✓ HasKey works" << std::endl;

    // 测试GetAllSections
    auto sections = config.GetAllSections();
    assert(sections.size() == 2);
    std::cout << "✓ GetAllSections works" << std::endl;

    // 测试GetAllKeys
    auto keys = config.GetAllKeys("Section1");
    assert(keys.size() == 2);
    std::cout << "✓ GetAllKeys works" << std::endl;
}

// 测试删除功能
void testRemoveFunctions() {
    std::cout << "\n=== Testing Remove Functions ===" << std::endl;

    bre::Ini& config = bre::Ini::Instance("test_remove.ini", true);
    config.Set("TestSection", "Key1", "Value1");
    config.Set("TestSection", "Key2", "Value2");
    config.Set("TestSection2", "Key3", "Value3");

    // 测试删除key
    assert(config.HasKey("TestSection", "Key1") == true);
    bool removed = config.RemoveKey("TestSection", "Key1");
    assert(removed == true);
    assert(config.HasKey("TestSection", "Key1") == false);
    std::cout << "✓ RemoveKey works" << std::endl;

    // 测试删除section
    assert(config.HasSection("TestSection2") == true);
    bool sectionRemoved = config.RemoveSection("TestSection2");
    assert(sectionRemoved == true);
    assert(config.HasSection("TestSection2") == false);
    std::cout << "✓ RemoveSection works" << std::endl;
}

// 主测试函数
int testIni() {
    std::cout << "======================================" << std::endl;
    std::cout << "  INI Config Load Comprehensive Test  " << std::endl;
    std::cout << "======================================" << std::endl;

    try {
        testIniExample();
        testTypeConversion();
        testCommentsAndSpecialChars();
        testModifyAndSave();
        testQueryFunctions();
        testRemoveFunctions();

        std::cout << "\n======================================" << std::endl;
        std::cout << "  ✓ All tests passed successfully!   " << std::endl;
        std::cout << "======================================" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
