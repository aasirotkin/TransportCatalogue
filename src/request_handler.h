#pragma once

#include "json.h"
#include "json_builder.h"
#include "geo.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <istream>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

namespace request_handler {

// ---------- RequestHandler --------------------------------------------------

class RequestHandler {
public:
    RequestHandler(transport_catalogue::TransportCatalogue& catalogue);

    // ����� ��������� ����� ���������
    void AddStop(std::string&& name, Coordinates&& coord);

    // ����� ��������� �������� ��������� ����� ����� �����������
    void AddDistance(std::string_view name_from, std::string_view name_to, double distance);

    // ����� ��������� ����� �������
    void AddBus(std::string&& name, std::vector<std::string_view>&& route, transport_catalogue::RouteType type);

    // ����� �������������� ���������� � ��������� ����� ��������� � svg �������
    void RenderMap(map_renderer::MapRendererSettings&& settings);

    // ����� ���������� ������ ���������, ���������� ����� �������� ���������
    const std::pair<std::set<std::string_view>, bool> GetStopBuses(std::string_view name) const {
        return catalogue_.GetBusesForStop(name);
    }

    // ����� ���������� �������� �������
    const transport_catalogue::detail::VirtualPair<transport_catalogue::bus_catalogue::Bus> GetBus(std::string_view name) const {
        return catalogue_.GetBus(name);
    }

    // ����� ���������� ������������������ ����� ���������� � ������ ��������� � svg �������
    const std::optional<std::string>& GetMap() const {
        return map_renderer_value_;
    }

private:
    transport_catalogue::TransportCatalogue& catalogue_;
    std::optional<std::string> map_renderer_value_;
};

// ----------------------------------------------------------------------------

namespace detail_base {

// ������� ������������ ������ �� �������� ���������
void RequestBaseStopProcess(
    RequestHandler& request_handler,
    const json::Node* node);

// ������� ������������ ������ �� �������� ����������� ��������
void RequestBaseBusProcess(
    RequestHandler& request_handler,
    const json::Node* node);

// ������� ����������� json ���� � ����
svg::Color ParseColor(const json::Node* node);

// ������� ����������� json ������ � ����� ������
std::vector<svg::Color> ParsePaletteColors(const json::Array& array_color_palette);

// ������� ����������� json ������ � ���������� ����� �� ���������
svg::Point ParseOffset(const json::Array& offset);

// ������� ������������ ������ �� �������� ����� ���������
void RequestBaseMapProcess(
    RequestHandler& request_handler,
    const std::unordered_map<std::string_view, const json::Node*>& render_settings);

} // namespace detail_base

// ----------------------------------------------------------------------------

namespace detail_stat {

// ������� ������������ ������ �� ��������� ���������� �� ���������
void RequestStatStopProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Dict& request);

// ������� ������������ ������ �� ��������� ���������� � ��������
void RequestStatBusProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Dict& request);

// ������� ������������ ������ �� ��������� ����� ���������
void RequestMapProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Dict& request);

// ������� �������������� ��� �������
void RequestStatProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Node* node);

} // namespace detail_stat

// ----------------------------------------------------------------------------

void RequestHandlerProcess(std::istream& input, std::ostream& output);

} // namespace request_handler
