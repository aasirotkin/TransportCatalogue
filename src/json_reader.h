#pragma once

#include "json.h"

#include <iostream>

namespace json {

// ---------- Document --------------------------------------------------------

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator== (const Document& lhs, const Document& rhs);

bool operator!= (const Document& lhs, const Document& rhs);

// ----------------------------------------------------------------------------

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

void Print(std::string_view str, std::ostream& output);

} // namespace json
