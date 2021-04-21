#pragma once

#include "json.h"

#include <iostream>
#include <unordered_map>
#include <vector>

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

// ---------- Reader ----------------------------------------------------------

class Reader {
public:
    explicit Reader(std::istream& input);

    const std::vector<const json::Node*>& StopRequests() const {
        return stop_requests_;
    }

    const std::vector<const json::Node*>& BusRequests() const {
        return bus_requests_;
    }

    const std::vector<const json::Node*>& StatRequests() const {
        return stat_requests_;
    }

    const std::unordered_map<std::string_view, const json::Node*>& RenderSettings() const {
        return render_settings_;
    }

    const std::unordered_map<std::string_view, const json::Dict*>& RoadDistances() const {
        return road_distances_;
    }

    const std::unordered_map<std::string_view, const json::Node*>& RoutingSettings() const {
        return routing_settings_;
    }

    const std::unordered_map<std::string_view, const json::Node*>& SerializationSettings() const {
        return serialization_settings_;
    }

private:
    void InitBaseRequests(const json::Array& base_requests);
    void InitStatRequests(const json::Array& stat_requests);
    void InitRenderSettings(const json::Dict& input_requests);
    void InitRoutingSettings(const json::Dict& routing_settings);
    void InitSerializationSettings(const json::Dict& serialization_settings);

private:
    json::Document doc_;

    // ����������, ���������� ������� �� �������� ���������
    std::vector<const json::Node*> stop_requests_;

    // ����������, ���������� ������� �� �������� ���������� ���������
    std::vector<const json::Node*> bus_requests_;

    // ����������, ���������� ������� �� ��������� ���������� �� ������������� ��������
    std::vector<const json::Node*> stat_requests_;

    // ����������, ���������� ��������� ����������� ����� ���������
    std::unordered_map<std::string_view, const json::Node*> render_settings_;

    // �������������� ���������� � �������� �� �������� ���������, �������� �������� �������� ���������� ����� �����������
    std::unordered_map<std::string_view, const json::Dict*> road_distances_;

    // ����������, ���������� ��������� ��������
    std::unordered_map<std::string_view, const json::Node*> routing_settings_;

    // ����������, ���������� ��������� ������������
    std::unordered_map<std::string_view, const json::Node*> serialization_settings_;
};

} // namespace json
