#pragma once

#include <istream>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace json {

// ---------- ParsingError ----------------------------------------------------

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

// ----------------------------------------------------------------------------

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// ---------- NodePrinter -----------------------------------------------------

struct NodePrinter {
    void operator() (std::nullptr_t) const;
    void operator() (const std::string& value) const;
    void operator() (bool value) const;
    void operator() (int value) const;
    void operator() (double value) const;
    void operator() (const Array& value) const;
    void operator() (const Dict& value) const;

    std::ostream& out;
};

// ---------- Node ------------------------------------------------------------

class Node : private std::variant<std::nullptr_t, std::string, bool, int, double, Array, Dict> {
public:
    using variant::variant;
    using NodeData = variant;
    using Value = variant;

public:
    const std::string& AsString() const;
    bool AsBool() const;
    int AsInt() const;
    double AsDouble() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    const Dict& AsDict() const;

    bool IsNull() const;
    bool IsString() const;
    bool IsBool() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsArray() const;
    bool IsMap() const;
    bool IsDict() const;

    const NodeData& Data() const;
};

bool operator== (const Node& lhs, const Node& rhs);

bool operator!= (const Node& lhs, const Node& rhs);

} // namespace json
