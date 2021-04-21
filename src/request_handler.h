#pragma once

#include "json.h"
#include "json_builder.h"
#include "geo.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <istream>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

namespace request_handler {

enum class ProgrammType {
    MAKE_BASE,
    PROCESS_REQUESTS,
    UNKNOWN
};

ProgrammType ParseProgrammType(int argc, const char** argv);

// ---------- RequestHandler --------------------------------------------------

class RequestHandler {
private:
    using RouteData = transport_graph::TransportRouter::TransportRouterData;

public:
    RequestHandler(transport_catalogue::TransportCatalogue& catalogue);

    // ����� ��������� ����� ���������
    void AddStop(std::string&& name, Coordinates&& coord);

    // ����� ��������� �������� ��������� ����� ����� �����������
    void AddDistance(std::string_view name_from, std::string_view name_to, double distance);

    // ����� ��������� ����� �������
    void AddBus(transport_catalogue::bus_catalogue::BusHelper&& bus_helper);

    // ����� �������������� ���������� � ��������� ����� ��������� � svg �������
    void RenderMap(map_renderer::MapRendererSettings&& settings);

    // ����� ��������� ������������ ��������
    bool IsRouteValid(
        const transport_catalogue::stop_catalogue::Stop* from,
        const transport_catalogue::stop_catalogue::Stop* to,
        const transport_catalogue::bus_catalogue::Bus* bus,
        int span, double time) const;

    // ����� ��������� ���������� �� ��������� � �������� ������
    bool DoesStopExist(std::string_view name) const {
        return catalogue_.GetStops().At(name) != std::nullopt;
    }

    // ����� ���������� ������ ���������, ���������� ����� �������� ���������
    const std::set<std::string_view>& GetStopBuses(std::string_view name) const {
        return catalogue_.GetBusesForStop(name);
    }

    // ����� ���������� �������� �������
    const std::optional<const transport_catalogue::bus_catalogue::Bus*> GetBus(std::string_view name) const {
        return catalogue_.GetBuses().At(name);
    }

    // ����� ���������� ������������������ ����� ���������� � ������ ��������� � svg �������
    const std::optional<std::string>& GetMap() const {
        return map_renderer_value_;
    }

    // ����� ���������� ��������� ����������� ����� ���������
    const std::optional<map_renderer::MapRendererSettings>& GetMapRenderSettings() const {
        return map_render_settings_;
    }

    // ����� ���������� ������ �������� �� ��������� from �� ��������� to
    std::optional<RouteData> GetRoute(std::string_view from, std::string_view to) const;

private:
    transport_catalogue::TransportCatalogue& catalogue_;
    std::optional<std::string> map_renderer_value_;
    std::optional<map_renderer::MapRendererSettings> map_render_settings_;
    mutable std::unique_ptr<transport_graph::TransportGraph> graph_;
    mutable std::unique_ptr<transport_graph::TransportRouter> router_;
};

// ----------------------------------------------------------------------------

namespace detail_base {

// ������� ������������ ������ �� �������� ���������
void RequestBaseStopProcess(
    RequestHandler& request_handler,
    const json::Node* node);

// ������� ������������ ������ �� �������� ����������� ��������
transport_catalogue::bus_catalogue::BusHelper RequestBaseBusProcess(const json::Node* node);

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

void RequestHandlerMakeBaseProcess(std::istream& input);

void RequestHandlerProcessRequestProcess(std::istream& input, std::ostream& output);

} // namespace request_handler
