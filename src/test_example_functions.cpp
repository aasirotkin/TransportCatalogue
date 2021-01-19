#include "test_example_functions.h"

#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

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

// Функция TestQueriesEngine является точкой входа прохождения тестов платформы
void TestQueriesEngine(std::istream& input_ss, std::ostream& output_ss) {
    TransportCatalogue catalogue;

    int query_count = 0;

    query_count = ReadLineWithNumber(input_ss);
    std::vector<std::string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        std::string query = ReadLine(input_ss);
        queries.push_back(query);
    }
    InputQueries(catalogue, queries);

    query_count = ReadLineWithNumber(input_ss);
    for (int i = 0; i < query_count; ++i) {
        std::string query = ReadLine(input_ss);
        OutputQuery(catalogue, query, output_ss);
    }
}

void TestQueries(const vector<string>& input, const vector<string>& output) {
    stringstream input_ss;
    for (const string& input_i : input) {
        input_ss << input_i;
    }

    stringstream output_ss;
    for (const string& output_i : output) {
        output_ss << output_i;
    }

    stringstream result_ss;
    TestQueriesEngine(input_ss, result_ss);

    string result_string;
    string output_string;

    ASSERT_EQUAL(result_ss.str().size(), output_ss.str().size());
    while (getline(result_ss, result_string) && getline(output_ss, output_string)) {
        ASSERT_EQUAL(result_string, output_string);
    }
}

void TestPlatformQueries() {
    const vector<string>& input = {
        "10\n"s,
        "Stop Tolstopaltsevo: 55.611087, 37.208290\n"s,
        "Stop Marushkino: 55.595884, 37.209755\n"s,
        "Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye\n"s,
        "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"s,
        "Stop Rasskazovka: 55.632761, 37.333324\n"s,
        "Stop Biryulyovo Zapadnoye: 55.574371, 37.651700\n"s,
        "Stop Biryusinka: 55.581065, 37.648390\n"s,
        "Stop Universam: 55.587655, 37.645687\n"s,
        "Stop Biryulyovo Tovarnaya: 55.592028, 37.653656\n"s,
        "Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164\n"s,
        "3\n",
        "Bus 256\n"s,
        "Bus 750\n"s,
        "Bus 751\n"s,
    };

    const vector<string>& output = {
        "Bus 256: 6 stops on route, 5 unique stops, 4371.02 route length\n"s,
        "Bus 750: 5 stops on route, 3 unique stops, 20939.5 route length\n"s,
        "Bus 751: not found\n"s
    };

    TestQueries(input, output);
}

// --------- Окончание модульных тестов -----------

// Функция TestTransportCatalogue является точкой входа для запуска тестов
void TestTransportCatalogue() {
    RUN_TEST(TestParseGeoFromStringView);
    RUN_TEST(TestPlatformQueries);
#ifndef _DEBUG

#endif
}
