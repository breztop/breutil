#include <boost/test/unit_test.hpp>
#include <cmath>

#include "breutil/flag.hpp"

using namespace bre;

BOOST_AUTO_TEST_SUITE(FlagTestSuite)

BOOST_AUTO_TEST_CASE(test_parse_basic) {
    const char* argv[] = {"test", "-name", "Alice", "-age", "30", "-verbose"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "Basic Test");

    flag.Add("name", std::string("default_name"), "User name");
    flag.Add("age", 0, "User age");
    flag.Add("verbose", false, "Verbose output");
    flag.Add("port", 8080, "Server port (optional)");

    flag.Parse();

    std::string name;
    int age;
    bool verbose;
    int port;

    flag.Get("name", name);
    flag.Get("age", age);
    flag.Get("verbose", verbose);
    flag.Get("port", port);

    BOOST_CHECK_EQUAL(name, "Alice");
    BOOST_CHECK_EQUAL(age, 30);
    BOOST_CHECK_EQUAL(verbose, true);
    BOOST_CHECK_EQUAL(port, 8080);
}

BOOST_AUTO_TEST_CASE(test_parse_equals) {
    const char* argv[] = {"test", "-host=localhost", "-port=9000", "-enabled=true"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "Equals Test");

    flag.Add("host", std::string("127.0.0.1"), "Server host");
    flag.Add("port", 8080, "Server port");
    flag.Add("enabled", false, "Enable feature");

    flag.Parse();

    std::string host;
    int port;
    bool enabled;

    flag.Get("host", host);
    flag.Get("port", port);
    flag.Get("enabled", enabled);

    BOOST_CHECK_EQUAL(host, "localhost");
    BOOST_CHECK_EQUAL(port, 9000);
    BOOST_CHECK_EQUAL(enabled, true);
}

BOOST_AUTO_TEST_CASE(test_parse_float) {
    const char* argv[] = {"test", "-pi", "3.14159", "-rate=0.05"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "Float Test");

    flag.Add("pi", 3.14f, "Pi value");
    flag.Add("rate", 0.01, "Rate value");

    flag.Parse();

    float pi;
    double rate;

    flag.Get("pi", pi);
    flag.Get("rate", rate);

    BOOST_CHECK_SMALL(std::abs(pi - 3.14159f), 0.00001f);
    BOOST_CHECK_SMALL(std::abs(rate - 0.05), 0.000001);
}

BOOST_AUTO_TEST_CASE(test_parse_positional) {
    const char* argv[] = {"test", "-verbose", "file1.txt", "file2.txt", "file3.txt"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "Positional Test");

    flag.Add("verbose", false, "Verbose output");
    flag.Parse();

    bool verbose;
    flag.Get("verbose", verbose);

    BOOST_CHECK_EQUAL(verbose, true);
    BOOST_CHECK_EQUAL(flag.Narg(), 3);

    auto args = flag.Arg();
    BOOST_CHECK_EQUAL(args.size(), 3);
    BOOST_CHECK_EQUAL(args[0], "file1.txt");
    BOOST_CHECK_EQUAL(args[1], "file2.txt");
    BOOST_CHECK_EQUAL(args[2], "file3.txt");
}

BOOST_AUTO_TEST_CASE(test_parse_all_types) {
    const char* argv[] = {"test",   "-bool1", "true",    "-bool2", "-int",
                          "42",     "-uint",  "100",     "-int64", "9223372036854775807",
                          "-float", "2.71",   "-double", "1.414",  "-string",
                          "hello"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "All Types Test");

    flag.Add("bool1", false, "Bool flag 1");
    flag.Add("bool2", false, "Bool flag 2");
    flag.Add("int", 0, "Int flag");
    flag.Add("uint", 0u, "Uint flag");
    flag.Add("int64", static_cast<int64_t>(0), "Int64 flag");
    flag.Add("float", 0.0f, "Float flag");
    flag.Add("double", 0.0, "Double flag");
    flag.Add("string", std::string(""), "String flag");

    flag.Parse();

    bool bool1, bool2;
    int int_val;
    unsigned int uint_val;
    int64_t int64_val;
    float float_val;
    double double_val;
    std::string string_val;

    flag.Get("bool1", bool1);
    flag.Get("bool2", bool2);
    flag.Get("int", int_val);
    flag.Get("uint", uint_val);
    flag.Get("int64", int64_val);
    flag.Get("float", float_val);
    flag.Get("double", double_val);
    flag.Get("string", string_val);

    BOOST_CHECK_EQUAL(bool1, true);
    BOOST_CHECK_EQUAL(bool2, true);
    BOOST_CHECK_EQUAL(int_val, 42);
    BOOST_CHECK_EQUAL(uint_val, 100);
    BOOST_CHECK_EQUAL(int64_val, 9223372036854775807LL);
    BOOST_CHECK_SMALL(std::abs(float_val - 2.71f), 0.01f);
    BOOST_CHECK_SMALL(std::abs(double_val - 1.414), 0.001);
    BOOST_CHECK_EQUAL(string_val, "hello");
}

BOOST_AUTO_TEST_CASE(test_parse_double_dash) {
    const char* argv[] = {"test", "--name=Bob", "--count", "5"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "Double Dash Test");

    flag.Add("name", std::string("default"), "Name");
    flag.Add("count", 0, "Count");

    flag.Parse();

    std::string name;
    int count;
    flag.Get("name", name);
    flag.Get("count", count);

    BOOST_CHECK_EQUAL(name, "Bob");
    BOOST_CHECK_EQUAL(count, 5);
}

BOOST_AUTO_TEST_CASE(test_bool_formats) {
    const char* argv[] = {"test",     "-flag1=1",    "-flag2=0",    "-flag3=t",
                          "-flag4=F", "-flag5=TRUE", "-flag6=False"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "Bool Formats Test");

    flag.Add("flag1", false, "Flag 1");
    flag.Add("flag2", false, "Flag 2");
    flag.Add("flag3", false, "Flag 3");
    flag.Add("flag4", true, "Flag 4");
    flag.Add("flag5", false, "Flag 5");
    flag.Add("flag6", true, "Flag 6");

    flag.Parse();

    bool f1, f2, f3, f4, f5, f6;
    flag.Get("flag1", f1);
    flag.Get("flag2", f2);
    flag.Get("flag3", f3);
    flag.Get("flag4", f4);
    flag.Get("flag5", f5);
    flag.Get("flag6", f6);

    BOOST_CHECK_EQUAL(f1, true);
    BOOST_CHECK_EQUAL(f2, false);
    BOOST_CHECK_EQUAL(f3, true);
    BOOST_CHECK_EQUAL(f4, false);
    BOOST_CHECK_EQUAL(f5, true);
    BOOST_CHECK_EQUAL(f6, false);
}

BOOST_AUTO_TEST_CASE(test_default_values) {
    const char* argv[] = {"test"};
    int argc = 1;

    Flag flag(argc, (char**)argv, "Default Values Test");

    flag.Add("name", std::string("DefaultName"), "Name");
    flag.Add("port", 8080, "Port");
    flag.Add("enabled", true, "Enabled");
    flag.Add("ratio", 0.5, "Ratio");

    flag.Parse();

    std::string name;
    int port;
    bool enabled;
    double ratio;

    flag.Get("name", name);
    flag.Get("port", port);
    flag.Get("enabled", enabled);
    flag.Get("ratio", ratio);

    BOOST_CHECK_EQUAL(name, "DefaultName");
    BOOST_CHECK_EQUAL(port, 8080);
    BOOST_CHECK_EQUAL(enabled, true);
    BOOST_CHECK_SMALL(std::abs(ratio - 0.5), 0.0001);
}

BOOST_AUTO_TEST_CASE(test_mixed_args) {
    const char* argv[] = {"test",       "input.txt",  "-verbose",  "-output",
                          "result.txt", "extra1.dat", "extra2.dat"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "Mixed Args Test");

    flag.Add("verbose", false, "Verbose");
    flag.Add("output", "", "Output file", true);

    flag.Parse();

    bool verbose;
    std::string output;
    flag.Get("verbose", verbose);
    flag.Get("output", output);

    BOOST_CHECK_EQUAL(verbose, true);
    BOOST_CHECK_EQUAL(output, "result.txt");
    BOOST_CHECK_EQUAL(flag.Narg(), 3);

    auto args = flag.Arg();
    BOOST_CHECK_EQUAL(args[0], "input.txt");
    BOOST_CHECK_EQUAL(args[1], "extra1.dat");
    BOOST_CHECK_EQUAL(args[2], "extra2.dat");
}

BOOST_AUTO_TEST_CASE(test_negative_numbers) {
    const char* argv[] = {"test", "-temp", "-25", "-balance=-100.50"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "Negative Numbers Test");

    flag.Add("temp", 0, "Temperature");
    flag.Add("balance", 0.0, "Balance");

    flag.Parse();

    int temp;
    double balance;
    flag.Get("temp", temp);
    flag.Get("balance", balance);

    BOOST_CHECK_EQUAL(temp, -25);
    BOOST_CHECK_SMALL(std::abs(balance - (-100.50)), 0.01);
}

BOOST_AUTO_TEST_CASE(test_arg_index) {
    const char* argv[] = {"test", "first", "second", "third"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "Arg Index Test");
    flag.Parse();

    BOOST_CHECK_EQUAL(flag.Narg(), 3);
    BOOST_CHECK_EQUAL(flag.Arg(1), "first");
    BOOST_CHECK_EQUAL(flag.Arg(2), "second");
    BOOST_CHECK_EQUAL(flag.Arg(3), "third");
    BOOST_CHECK_EQUAL(flag.Arg(0), "");
    BOOST_CHECK_EQUAL(flag.Arg(4), "");
}

BOOST_AUTO_TEST_CASE(test_parsed_state) {
    const char* argv[] = {"test", "-name", "Test"};
    int argc = 3;

    Flag flag(argc, (char**)argv, "Parsed State Test");
    flag.Add("name", std::string("default"), "Name");

    BOOST_CHECK_EQUAL(flag.Parsed(), false);
    flag.Parse();
    BOOST_CHECK_EQUAL(flag.Parsed(), true);
}

BOOST_AUTO_TEST_CASE(test_empty_string) {
    const char* argv[] = {"test", "-message="};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "Empty String Test");
    flag.Add("message", std::string("default"), "Message");

    flag.Parse();

    std::string message;
    flag.Get("message", message);

    BOOST_CHECK_EQUAL(message, "");
}

BOOST_AUTO_TEST_CASE(test_nflag) {
    const char* argv[] = {"test"};
    int argc = 1;

    Flag flag(argc, (char**)argv, "Nflag Test");

    BOOST_CHECK_EQUAL(flag.Nflag(), 0);

    flag.Add("flag1", false, "Flag 1");
    BOOST_CHECK_EQUAL(flag.Nflag(), 1);

    flag.Add("flag2", 0, "Flag 2");
    BOOST_CHECK_EQUAL(flag.Nflag(), 2);

    flag.Add("flag3", std::string(""), "Flag 3");
    BOOST_CHECK_EQUAL(flag.Nflag(), 3);
}

BOOST_AUTO_TEST_CASE(test_required_flags) {
    const char* argv[] = {"test", "-username", "alice", "-password", "secret123"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    Flag flag(argc, (char**)argv, "Required Flags Test");

    flag.Add("username", "", "Username", true);
    flag.Add("password", "", "Password", true);
    flag.Add("port", 8080, "Port");

    flag.Parse();

    std::string username, password;
    int port;
    flag.Get("username", username);
    flag.Get("password", password);
    flag.Get("port", port);

    BOOST_CHECK_EQUAL(username, "alice");
    BOOST_CHECK_EQUAL(password, "secret123");
    BOOST_CHECK_EQUAL(port, 8080);
}

BOOST_AUTO_TEST_SUITE_END()
