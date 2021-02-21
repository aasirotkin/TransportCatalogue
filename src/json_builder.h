#pragma once

#include "json.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace json {

// ----------------------------------------------------------------------------

namespace detail {

struct NodeGetter {
    Node operator() (std::nullptr_t) const;
    Node operator() (std::string&& value) const;
    Node operator() (bool&& value) const;
    Node operator() (int&& value) const;
    Node operator() (double&& value) const;
    Node operator() (Array&& value) const;
    Node operator() (Dict&& value) const;
};

} // namespace detail

// ----------------------------------------------------------------------------

class DictItemContext;
class KeyItemContext;
class ArrayItemContext;

// ----------------------------------------------------------------------------

class Builder {
public:
    Builder() = default;

    // ����� ��������� �������� ����� ��� ��������� ���� ����-��������
    KeyItemContext Key(std::string value);

    // ����� ��������, ��������������� ����� ��� ����������� �������, ��������� ������� ������� �� ���������� ��������������� JSON-�������
    Builder& Value(Node::Value value);

    // �������� ����������� �������� ��������-�������
    DictItemContext StartDict();

    // �������� ����������� �������� ��������-�������
    ArrayItemContext StartArray();

    // ��������� ����������� �������� ��������-�������
    Builder& EndDict();

    // ��������� ����������� �������� ��������-�������
    Builder& EndArray();

    // ���������� ������ json::Node, ���������� JSON, ��������� ����������� �������� �������
    Node Build();

private:
    Node root_;
    std::vector<std::unique_ptr<Node>> nodes_stack_;
    int array_counter_ = 0;
    int dict_counter_ = 0;
};

// ----------------------------------------------------------------------------

class ItemContext {
public:
    ItemContext(Builder& builder)
        : builder_(builder) {
    }

protected:
    Builder& Get() {
        return builder_;
    }

private:
    Builder& builder_;
};

// ----------------------------------------------------------------------------

class KeyItemContext : public ItemContext {
public:
    KeyItemContext(Builder& builder)
        : ItemContext(builder) {
    }

    DictItemContext Value(Node::Value value);

    DictItemContext StartDict();

    ArrayItemContext StartArray();
};

// ----------------------------------------------------------------------------

class DictItemContext : public ItemContext {
public:
    DictItemContext(Builder& builder)
        : ItemContext(builder) {
    }

    KeyItemContext Key(std::string value);

    Builder& EndDict();
};

// ----------------------------------------------------------------------------

class ArrayItemContext : public ItemContext {
public:
    ArrayItemContext(Builder& builder)
        : ItemContext(builder) {
    }

    ArrayItemContext Value(Node::Value value);

    DictItemContext StartDict();

    ArrayItemContext StartArray();

    Builder& EndArray();
};

// ----------------------------------------------------------------------------

} // namespace json
