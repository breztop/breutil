#include <boost/test/unit_test.hpp>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "breutil/ini.hpp"

using namespace bre;

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR ""
#endif

BOOST_AUTO_TEST_SUITE(IniTestSuite)

BOOST_AUTO_TEST_CASE(test_ini_example) {
    const std::filesystem::path dataPath = std::filesystem::path(TEST_DATA_DIR) / "ini_example.ini";

    bre::Ini& config = bre::Ini::Instance(dataPath.string());

    std::string host = config.GetStr("Database", "Host", "");
    int port = config.GetInt("Database", "Port", 0);
    std::string username = config.GetStr("Database", "Username", "");
    std::string password = config.GetStr("Database", "Password", "");
    std::string database = config.GetStr("Database", "Database", "");
    int maxConnections = config.GetInt("Database", "MaxConnections", 0);
    double timeout = config.GetDouble("Database", "Timeout", 0.0);
    bool enableSSL = config.GetBool("Database", "EnableSSL", false);
    bool autoReconnect = config.GetBool("Database", "AutoReconnect", false);

    BOOST_CHECK_EQUAL(host, "localhost");
    BOOST_CHECK_EQUAL(port, 3306);
    BOOST_CHECK_EQUAL(username, "admin");
    BOOST_CHECK_EQUAL(password, "my_secure_password");
    BOOST_CHECK_EQUAL(database, "mydb");
    BOOST_CHECK_EQUAL(maxConnections, 100);
    BOOST_CHECK_SMALL(std::abs(timeout - 30.5), 0.01);
    BOOST_CHECK_EQUAL(enableSSL, true);
    BOOST_CHECK_EQUAL(autoReconnect, true);

    std::string listenAddr = config.GetStr("Server", "ListenAddress", "");
    int listenPort = config.GetInt("Server", "ListenPort", 0);
    int maxThreads = config.GetInt("Server", "MaxThreads", 0);
    bool debug = config.GetBool("Server", "Debug", true);
    std::string logLevel = config.GetStr("Server", "LogLevel", "");
    std::string rootPath = config.GetStr("Server", "RootPath", "");

    BOOST_CHECK_EQUAL(listenAddr, "0.0.0.0");
    BOOST_CHECK_EQUAL(listenPort, 5432);
    BOOST_CHECK_EQUAL(maxThreads, 16);
    BOOST_CHECK_EQUAL(debug, false);
    BOOST_CHECK_EQUAL(logLevel, "INFO");
    BOOST_CHECK_EQUAL(rootPath, "/var/www/html");

    std::string appName = config.GetStr("Application", "Name", "");
    std::string version = config.GetStr("Application", "Version", "");
    std::string author = config.GetStr("Application", "Author", "");
    bool featureX = config.GetBool("Application", "EnableFeatureX", false);
    bool featureY = config.GetBool("Application", "EnableFeatureY", true);

    BOOST_CHECK_EQUAL(appName, "My Application");
    BOOST_CHECK_EQUAL(version, "1.2.3");
    BOOST_CHECK_EQUAL(author, "John Doe");
    BOOST_CHECK_EQUAL(featureX, true);
    BOOST_CHECK_EQUAL(featureY, false);

    bool cacheEnabled = config.GetBool("Cache", "Enabled", false);
    std::string cacheType = config.GetStr("Cache", "Type", "");
    std::string cacheHost = config.GetStr("Cache", "Host", "");
    int cachePort = config.GetInt("Cache", "Port", 0);
    int ttl = config.GetInt("Cache", "TTL", 0);
    int maxMemory = config.GetInt("Cache", "MaxMemory", 0);

    BOOST_CHECK_EQUAL(cacheEnabled, true);
    BOOST_CHECK_EQUAL(cacheType, "redis");
    BOOST_CHECK_EQUAL(cacheHost, "127.0.0.1");
    BOOST_CHECK_EQUAL(cachePort, 6379);
    BOOST_CHECK_EQUAL(ttl, 3600);
    BOOST_CHECK_EQUAL(maxMemory, 512);

    std::string emptyValue = config.GetStr("Defaults", "EmptyValue", "default");
    std::string nullValue = config.GetStr("Defaults", "NullValue", "default");
    int zeroInt = config.GetInt("Defaults", "ZeroInt", -1);
    double zeroFloat = config.GetDouble("Defaults", "ZeroFloat", -1.0);
    bool falseValue = config.GetBool("Defaults", "FalseValue", true);

    BOOST_CHECK((emptyValue == "" || emptyValue == "default"));
    BOOST_CHECK_EQUAL(nullValue, "");
    BOOST_CHECK_EQUAL(zeroInt, 0);
    BOOST_CHECK_SMALL(std::abs(zeroFloat - 0.0), 0.001);
    BOOST_CHECK_EQUAL(falseValue, false);
}

BOOST_AUTO_TEST_CASE(test_type_conversion) {
    std::ofstream testFile("./test_types.ini");
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

    BOOST_CHECK_EQUAL(config.GetInt("Numbers", "IntValue"), 42);
    BOOST_CHECK_EQUAL(config.GetInt("Numbers", "NegativeInt"), -100);
    BOOST_CHECK_SMALL(std::abs(config.GetDouble("Numbers", "DoubleValue") - 3.14159), 0.00001);
    BOOST_CHECK_SMALL(std::abs(config.GetDouble("Numbers", "NegativeDouble") - (-2.718)), 0.001);

    BOOST_CHECK_EQUAL(config.GetBool("Boolean", "BoolTrue1"), true);
    BOOST_CHECK_EQUAL(config.GetBool("Boolean", "BoolTrue2"), true);
    BOOST_CHECK_EQUAL(config.GetBool("Boolean", "BoolTrue3"), true);
    BOOST_CHECK_EQUAL(config.GetBool("Boolean", "BoolTrue4"), true);
    BOOST_CHECK_EQUAL(config.GetBool("Boolean", "BoolTrue5"), true);

    BOOST_CHECK_EQUAL(config.GetBool("Boolean", "BoolFalse1"), false);
    BOOST_CHECK_EQUAL(config.GetBool("Boolean", "BoolFalse2"), false);
    BOOST_CHECK_EQUAL(config.GetBool("Boolean", "BoolFalse3"), false);
    BOOST_CHECK_EQUAL(config.GetBool("Boolean", "BoolFalse4"), false);
    BOOST_CHECK_EQUAL(config.GetBool("Boolean", "BoolFalse5"), false);

    BOOST_CHECK_EQUAL(config.GetInt("Invalid", "InvalidInt", 999), 999);
    BOOST_CHECK_SMALL(std::abs(config.GetDouble("Invalid", "InvalidDouble", 1.23) - 1.23),
                      0.000001);
}

BOOST_AUTO_TEST_CASE(test_comments_and_special_chars) {
    std::ofstream testFile("./test_special.ini");
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

    BOOST_CHECK_EQUAL(config.GetStr("Section1", "Key1"), "Value1");
    BOOST_CHECK_EQUAL(config.GetStr("Section1", "Key2"), "Value2");
    BOOST_CHECK_EQUAL(config.GetStr("Section1", "QuotedString"), "Hello World");
    BOOST_CHECK_EQUAL(config.GetStr("Section1", "SingleQuoted"), "Single Quote String");

    std::string escaped = config.GetStr("Section1", "EscapedChars");
    BOOST_CHECK(escaped.find('\n') != std::string::npos);
    BOOST_CHECK(escaped.find('\t') != std::string::npos);

    std::string path = config.GetStr("Section1", "PathWithBackslash");
    BOOST_CHECK((path == "C:\\Users\\Test") || (path == "C:\\\\Users\\\\Test"));
    BOOST_CHECK_EQUAL(config.GetStr("Section1", "EmptyQuotes"), "");
}

BOOST_AUTO_TEST_CASE(test_modify_and_save) {
    bre::Ini& config = bre::Ini::Instance("test_modify.ini", true);

    config.Set("NewSection", "StringValue", "TestString");
    config.Set("NewSection", "IntValue", 123);
    config.Set("NewSection", "DoubleValue", 45.67);
    config.Set("NewSection", "BoolValue", true);

    BOOST_CHECK_EQUAL(config.GetStr("NewSection", "StringValue"), "TestString");
    BOOST_CHECK_EQUAL(config.GetInt("NewSection", "IntValue"), 123);
    BOOST_CHECK_SMALL(std::abs(config.GetDouble("NewSection", "DoubleValue") - 45.67), 0.01);
    BOOST_CHECK_EQUAL(config.GetBool("NewSection", "BoolValue"), true);

    bool saved = config.Save();
    BOOST_CHECK_EQUAL(saved, true);

    bre::Ini& config2 = bre::Ini::Instance("./test_modify.ini");
    BOOST_CHECK_EQUAL(config2.GetStr("NewSection", "StringValue"), "TestString");
    BOOST_CHECK_EQUAL(config2.GetInt("NewSection", "IntValue"), 123);
}

BOOST_AUTO_TEST_CASE(test_query_functions) {
    std::ofstream testFile("./test_query.ini");
    testFile << "[Section1]\n";
    testFile << "Key1=Value1\n";
    testFile << "Key2=Value2\n";
    testFile << "\n[Section2]\n";
    testFile << "KeyA=ValueA\n";
    testFile.close();

    bre::Ini& config = bre::Ini::Instance("./test_query.ini");

    BOOST_CHECK_EQUAL(config.HasSection("Section1"), true);
    BOOST_CHECK_EQUAL(config.HasSection("Section2"), true);
    BOOST_CHECK_EQUAL(config.HasSection("NonExistent"), false);

    BOOST_CHECK_EQUAL(config.HasKey("Section1", "Key1"), true);
    BOOST_CHECK_EQUAL(config.HasKey("Section1", "Key2"), true);
    BOOST_CHECK_EQUAL(config.HasKey("Section1", "NonExistent"), false);
    BOOST_CHECK_EQUAL(config.HasKey("NonExistent", "Key1"), false);

    auto sections = config.GetAllSections();
    BOOST_CHECK_EQUAL(sections.size(), (size_t)2);

    auto keys = config.GetAllKeys("Section1");
    BOOST_CHECK_EQUAL(keys.size(), (size_t)2);
}

BOOST_AUTO_TEST_CASE(test_remove_functions) {
    bre::Ini& config = bre::Ini::Instance("test_remove.ini", true);
    config.Set("TestSection", "Key1", "Value1");
    config.Set("TestSection", "Key2", "Value2");
    config.Set("TestSection2", "Key3", "Value3");

    BOOST_CHECK_EQUAL(config.HasKey("TestSection", "Key1"), true);
    bool removed = config.RemoveKey("TestSection", "Key1");
    BOOST_CHECK_EQUAL(removed, true);
    BOOST_CHECK_EQUAL(config.HasKey("TestSection", "Key1"), false);

    BOOST_CHECK_EQUAL(config.HasSection("TestSection2"), true);
    bool sectionRemoved = config.RemoveSection("TestSection2");
    BOOST_CHECK_EQUAL(sectionRemoved, true);
    BOOST_CHECK_EQUAL(config.HasSection("TestSection2"), false);
}

BOOST_AUTO_TEST_SUITE_END()
