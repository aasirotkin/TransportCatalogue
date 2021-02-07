#include "json.h"

#include <utility>

namespace json {

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

Node::Node(std::string&& value)
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

Node::Node(Array&& value)
    : data_(std::move(value)) {
}

Node::Node(Dict&& value)
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

} // namespace json
