#include "test_example_functions.h"

#include "geo.h"
#include "request_handler.h"

#include <algorithm>
#include <chrono>
#include <deque>
#include <execution>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// -----------------------------------------------------------------------------

template <typename Type>
ostream& operator<< (ostream& out, const vector<Type>& vect) {
    out << "{ "s;
    for (auto it = vect.begin(); it != vect.end(); ++it) {
        if (it != vect.begin()) {
            out << ", ";
        }
        out << *it;
    }
    out << " }";
    return out;
}

// -----------------------------------------------------------------------------

template <typename Func>
void RunTestImpl(Func& func, const string& func_name) {
    func();
    cerr << func_name << " OK" << endl;
}

#define RUN_TEST(func) RunTestImpl((func), #func)

// -----------------------------------------------------------------------------

void AssertImpl(bool value, const string& value_str, const string& file,
                const string& func, unsigned line, const string& hint) {
    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << value_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT(a) AssertImpl((a), #a, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(a, hint) AssertImpl((a), #a, __FILE__, __FUNCTION__, __LINE__, hint)

// -----------------------------------------------------------------------------

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

// -----------------------------------------------------------------------------

enum class ErrorCode {
    OUT_OF_RANGE,
    INVALID_ARGUMENT
};

const string ErrorCodeName(ErrorCode code) {
    switch(code) {
    case ErrorCode::OUT_OF_RANGE: return "out_of_range"s;
    case ErrorCode::INVALID_ARGUMENT: return "invalid_argument"s;
    default: break;
    }
    ASSERT_HINT(false, "invalid exception"s);
    return "invalid exception"s;
}

const string ErrorCodeHint(ErrorCode code) {
    return "Must be "s + ErrorCodeName(code) + " exception"s;
}

template <typename Func>
void AssertTrowImpl(Func& func, ErrorCode code,
                    const string& file, const string& func_name, unsigned line) {
    try
    {
        func();
        AssertImpl(false, "Task failed successfully"s, file, func_name, line, ErrorCodeHint(code));
    }
    catch(const out_of_range&) {
        AssertImpl(code == ErrorCode::OUT_OF_RANGE, ErrorCodeName(code), file, func_name, line, ErrorCodeHint(code));
    }
    catch(const invalid_argument&) {
        AssertImpl(code == ErrorCode::INVALID_ARGUMENT, ErrorCodeName(code), file, func_name, line, ErrorCodeHint(code));
    }
    catch(...) {
        AssertImpl(false, ErrorCodeName(code), file, func_name, line, ErrorCodeHint(code));
    }
    cerr << func_name << " OK" << endl;
}

#define ASSERT_OUT_OF_RANGE(a) AssertTrowImpl((a), ErrorCode::OUT_OF_RANGE, __FILE__, #a, __LINE__)

#define ASSERT_INVALID_ARGUMENT(a) AssertTrowImpl((a), ErrorCode::INVALID_ARGUMENT, __FILE__, #a, __LINE__)

// -----------------------------------------------------------------------------

template <typename UnitOfTime>
class AssertDuration {
public:
    using Clock = std::chrono::steady_clock;

    AssertDuration(int64_t max_duration, const std::string file, const std::string function, unsigned line)
        : max_dur_(max_duration)
        , file_(file)
        , function_(function)
        , line_(line) {
    }

    ~AssertDuration() {
        const auto dur = Clock::now() - start_time_;
        const auto converted_dur = std::chrono::duration_cast<UnitOfTime>(dur).count();
        if (converted_dur > max_dur_) {
            cerr << "Assert duration fail: "s << file_ << " "s << function_ << ": "s << line_ << endl;
            cerr << "Process duration is "s << converted_dur << " while max duration is " << max_dur_ << endl;
            cerr << "So the function worked longer on "s << converted_dur - max_dur_ << endl;
            abort();
        }
    }

private:
    int64_t max_dur_;
    std::string file_;
    std::string function_;
    unsigned line_;
    const Clock::time_point start_time_ = Clock::now();
};

#define ASSERT_DURATION_MILLISECONDS(x) AssertDuration<std::chrono::milliseconds> UNIQUE_VAR_NAME_PROFILE(x, __FILE__, __FUNCTION__, __LINE__)
#define ASSERT_DURATION_SECONDS(x) AssertDuration<std::chrono::seconds> UNIQUE_VAR_NAME_PROFILE(x, __FILE__, __FUNCTION__, __LINE__)

// -----------------------------------------------------------------------------

// -------- Начало модульных тестов ----------

void TestParseGeoFromStringView() {
    Coordinates coord;
    {
        std::string str_coord = "55.611087, 37.208290"s;
        coord = Coordinates::ParseFromStringView(str_coord);
        ASSERT_EQUAL(coord, Coordinates({ 55.611087, 37.208290 }));
    }
    {
        std::string str_coord = "   55.611087   ,   37.208290\n"s;
        coord = Coordinates::ParseFromStringView(str_coord);
        ASSERT_EQUAL(coord, Coordinates({ 55.611087, 37.208290 }));
    }
    {
        std::string str_coord = "   55.611087   ,   37.208290    \n"s;
        coord = Coordinates::ParseFromStringView(str_coord);
        ASSERT_EQUAL(coord, Coordinates({ 55.611087, 37.208290 }));
    }
}

// ----------------------------------------------------------------------------

#include <algorithm>
#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <sstream>

struct TestData {
    std::string result;
    std::string reference;
};

void RemoveIndentInPlace(std::string& str) {
    str.erase(std::remove_if(str.begin(), str.end(), [](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }), str.end());
}

void TestsFromFile() {
    std::string path = "C:\\Users\\aasir\\source\\repos\\aasirotkin\\TransportCatalogue\\Tests\\";
    std::map<int, TestData> test_data;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::string file_name = entry.path().filename().u8string();

        std::ifstream t(path + file_name);
        std::stringstream buffer;
        buffer << t.rdbuf();

        size_t id_begin = file_name.find_first_of('_') + 1;
        int id = std::stoi(file_name.substr(id_begin));

        if (file_name.front() == 'i') {
            std::stringstream out;
            request_handler::RequestHandler(buffer, out);
            test_data[id].result = out.str();
        }
        else {
            test_data[id].reference = buffer.str();
        }
    }

    for (auto& [key, value] : test_data) {
        RemoveIndentInPlace(value.result);
        RemoveIndentInPlace(value.reference);

        ASSERT(value.result == value.reference);
    }
}

// ----------------------------------------------------------------------------

// --------- Окончание модульных тестов -----------

// Функция TestTransportCatalogue является точкой входа для запуска тестов
void TestTransportCatalogue() {
    RUN_TEST(TestParseGeoFromStringView);
    RUN_TEST(TestsFromFile);

#ifndef _DEBUG

#endif
}
