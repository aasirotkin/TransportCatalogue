#include "json.h"

#include <stdexcept>
#include <utility>

namespace json {

#define TO_STR(str) (std::string(str))

// ---------- NodePrinter -----------------------------------------------------

void NodePrinter::operator() (std::nullptr_t) const {
    using namespace std::literals;
    out << "null"sv;
}

void NodePrinter::operator() (const std::string& value) const {
    using namespace std::literals;

    static const std::map<char, std::string> escapes = {
        {'\"', "\\\""s},
        {'\\', "\\\\"s},
        {'\t', "\\t"s},
        {'\r', "\\r"s},
        {'\n', "\\n"s}
    };

    out << "\""sv;
    for (char c : value) {
        if (escapes.count(c)) {
            out << escapes.at(c);
        }
        else {
            out << c;
        }
    }
    out << "\""sv;
}

void NodePrinter::operator() (bool value) const {
    out << std::boolalpha << value;
}

void NodePrinter::operator() (int value) const {
    out << value;
}

void NodePrinter::operator() (double value) const {
    out << value;
}

void NodePrinter::operator() (const Array& value) const {
    using namespace std::literals;

    out << "[ "sv;
    bool first = true;
    for (const Node& node : value) {
        if (!first) {
            out << ", "sv;
        }
        std::visit(*this, node.Data());
        first = false;
    }
    out << " ]"sv;
}

void NodePrinter::operator() (const Dict& value) const {
    using namespace std::literals;

    out << "{ "sv;
    bool first = true;
    for (const auto& [name, node] : value) {
        if (!first) {
            out << ", "sv;
        }
        this->operator()(name);
        out << ": "sv;
        std::visit(*this, node.Data());
        first = false;
    }
    out << " }"sv;
}

// ---------- Node ------------------------------------------------------------

Node::Node(nullptr_t)
    : Node() {
}

Node::Node(std::string value)
    : data_(std::move(value)) {
}

Node::Node(bool value)
    : data_(value) {
}

Node::Node(int value)
    : data_(value) {
}

Node::Node(double value)
    : data_(value) {
}

Node::Node(Array value)
    : data_(std::move(value)) {
}

Node::Node(Dict value)
    : data_(std::move(value)) {
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Node data is not string format");
    }
    return std::get<std::string>(data_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Node data is not bool format");
    }
    return std::get<bool>(data_);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("Node data is not int format");
    }
    return std::get<int>(data_);
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error("Node data is not double format");
    }
    return (IsPureDouble()) ? std::get<double>(data_) : static_cast<double>(AsInt());
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Node data is not Array format");
    }
    return std::get<Array>(data_);
}

const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Node data is not Dict format");
    }
    return std::get<Dict>(data_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(data_);
}

bool Node::IsString() const {
    return std::holds_alternative<std::string>(data_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(data_);
}

bool Node::IsInt() const {
    return std::holds_alternative<int>(data_);
}

bool Node::IsDouble() const {
    return std::holds_alternative<double>(data_) || IsInt();
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(data_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(data_);
}

bool Node::IsMap() const {
    return std::holds_alternative<Dict>(data_);
}

const Node::NodeData& Node::Data() const {
    return data_;
}

bool operator== (const Node& lhs, const Node& rhs) {
    return lhs.Data() == rhs.Data();
}

bool operator!= (const Node& lhs, const Node& rhs) {
    return !(lhs.Data() == rhs.Data());
}

// ---------- Document --------------------------------------------------------

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool operator== (const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!= (const Document& lhs, const Document& rhs) {
    return !(lhs.GetRoot() == rhs.GetRoot());
}

// ----------------------------------------------------------------------------

Node LoadNode(std::istream& input);

std::string LoadStringCount(std::istream& input, size_t count) {
    std::string line;
    for (char c; input >> c && count > 0;) {
        line += c;
        count--;
    }
    return line;
}

bool LoadTypeCompare(std::istream& input, const std::string& check_word) {
    return check_word == LoadStringCount(input, check_word.size());
}

void LoadTypeCheck(std::istream& input, const std::string& check_word, const std::string& error_msg) {
    if (!LoadTypeCompare(input, check_word)) {
        throw ParsingError(error_msg);
    }
}

#define LOAD_NULL_CHECK(input) (LoadTypeCheck(input, TO_STR("null"), TO_STR("Json LoadNull error")))
#define LOAD_TRUE_CHECK(input) (LoadTypeCheck(input, TO_STR("true"), TO_STR("Json LoadTrue error")))
#define LOAD_FALSE_CHECK(input) (LoadTypeCheck(input, TO_STR("false"), TO_STR("Json LoadFalse error")))

Node LoadNull(std::istream& input) {
    LOAD_NULL_CHECK(input);
    return Node();
}

Node LoadString(std::istream& input) {
    static const std::map<char, char> escape{
        {'r', '\r'},
        {'n', '\n'},
        {'t', '\t'}
    };

    std::string line;

    input >> std::noskipws;

    bool is_closed = false;

    for (char c; input >> c;) {
        is_closed = (c == '\"');
        if (is_closed) {
            break;
        }
        if (c == '\\') {
            input >> c;
            if (escape.count(c)) {
                line += escape.at(c);
            }
            else {
                line += c;
            }
        }
        else {
            line += c;
        }
    }

    if (!is_closed) {
        throw ParsingError(TO_STR("Quote must be closed in string"));
    }

    return Node(move(line));
}

Node LoadTrue(std::istream& input) {
    LOAD_TRUE_CHECK(input);
    return Node(true);
}

Node LoadFalse(std::istream& input) {
    LOAD_FALSE_CHECK(input);
    return Node(false);
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    }
    else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(std::stoi(parsed_num));
            }
            catch (...) {
                // В случае неудачи, например, при переполнении
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(std::stod(parsed_num));
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadArray(std::istream& input) {
    Array result;

    bool is_closed = false;
    for (char c; input >> c;) {
        if (is_closed = (c == ']'); is_closed) {
            break;
        }
        if (c == ' ' || c == ',') {
            continue;
        }
        input.putback(c);
        result.push_back(LoadNode(input));
    }

    if (!is_closed) {
        throw ParsingError(TO_STR("Brackets must be closed in Array"));
    }

    return Node(move(result));
}

Node LoadDict(std::istream& input) {
    Dict result;

    bool is_closed = false;
    for (char c; input >> c;) {
        if (is_closed = (c == '}'); is_closed) {
            break;
        }
        if (c == ' ' || c == ',') {
            continue;
        }

        std::string key = LoadString(input).AsString();
        input >> c;
        result.insert({ std::move(key), LoadNode(input) });
    }

    if (!is_closed) {
        throw ParsingError(TO_STR("Brackets must be closed in Dict"));
    }

    return Node(std::move(result));
}

Node LoadNode(std::istream& input) {
    char c;
    input >> c;

    if (c == ' ') {
        return LoadNode(input);
    }
    else if (c == '[') {
        return LoadArray(input);
    }
    else if (c == '{') {
        return LoadDict(input);
    }
    else if (c == '"') {
        return LoadString(input);
    }
    else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    }
    else if (c == 't') {
        input.putback(c);
        return LoadTrue(input);
    }
    else if (c == 'f') {
        input.putback(c);
        return LoadFalse(input);
    }
    else {
        input.putback(c);
        return LoadNumber(input);
    }
}

// ----------------------------------------------------------------------------

Document Load(std::istream& input) {
    return Document{ LoadNode(input) };
}

void Print(const Document& doc, std::ostream& output) {
    std::visit(NodePrinter{ output }, doc.GetRoot().Data());
}

} // namespace json
