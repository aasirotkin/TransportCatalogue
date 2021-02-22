#include "request_handler.h"

#include "geo.h"
#include "json_reader.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace request_handler {

using namespace transport_catalogue;
using namespace map_renderer;

// ---------- RequestHandler --------------------------------------------------

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue& catalogue)
    : catalogue_(catalogue) {
}

void RequestHandler::AddStop(std::string&& name, Coordinates&& coord) {
    catalogue_.AddStop(std::move(name), std::move(coord));
}

void RequestHandler::AddDistance(std::string_view name_from, std::string_view name_to, double distance) {
    catalogue_.AddDistanceBetweenStops(name_from, name_to, distance);
}

void RequestHandler::AddBus(std::string&& name, std::vector<std::string_view>&& route, transport_catalogue::RouteType type) {
    catalogue_.AddBus(std::move(name), std::move(route), type);
}

void RequestHandler::RenderMap(MapRendererSettings&& settings) {
    MapRenderer render(
        std::move(settings),
        catalogue_.GetStopsCatalogue(),
        catalogue_.GetStops(),
        catalogue_.GetBuses());

    std::ostringstream oss;
    render.Render(oss);

    map_renderer_value_ = oss.str();
}

// ----------------------------------------------------------------------------

namespace detail_base {

void RequestBaseStopProcess(
    RequestHandler& request_handler,
    const json::Node* node) {
    using namespace std::literals;

    const json::Dict& request = node->AsMap();

    std::string name = request.at("name"s).AsString();

    request_handler.AddStop(std::move(name), Coordinates{ request.at("latitude"s).AsDouble(), request.at("longitude"s).AsDouble() });
}

void RequestBaseBusProcess(
    RequestHandler& request_handler,
    const json::Node* node) {
    using namespace std::literals;

    const json::Dict& request = node->AsMap();

    std::string name = request.at("name"s).AsString();

    transport_catalogue::RouteType type = (request.at("is_roundtrip"s).AsBool())
        ? transport_catalogue::RouteType::Round
        : transport_catalogue::RouteType::BackAndForth;

    std::vector<std::string_view> route;
    for (const json::Node& node_stops : request.at("stops"s).AsArray()) {
        route.push_back(node_stops.AsString());
    }

    request_handler.AddBus(std::string(name), std::move(route), type);
}

svg::Color ParseColor(const json::Node* node) {
    using namespace std::string_literals;

    if (node->IsString()) {
        return svg::Color(node->AsString());
    }
    else if (node->IsArray()) {
        const json::Array& array_color = node->AsArray();
        int r = array_color.at(0).AsInt();
        int g = array_color.at(1).AsInt();
        int b = array_color.at(2).AsInt();
        if (array_color.size() == 3) {
            return svg::Color(svg::Rgb(r, g, b));
        }
        else {
            double opacity = array_color.at(3).AsDouble();
            return svg::Color(svg::Rgba(r, g, b, opacity));
        }
    }
    throw json::ParsingError("ParseColor error"s);
    return svg::Color{};
}

std::vector<svg::Color> ParsePaletteColors(const json::Array& array_color_palette) {
    std::vector<svg::Color> color_palette;
    for (const json::Node& node : array_color_palette) {
        color_palette.push_back(std::move(ParseColor(&node)));
    }
    return color_palette;
}

svg::Point ParseOffset(const json::Array& offset) {
    return svg::Point{ offset.at(0).AsDouble(), offset.at(1).AsDouble() };
}

void RequestBaseMapProcess(
    RequestHandler& request_handler,
    const std::unordered_map<std::string_view, const json::Node*>& render_settings) {
    using namespace std::literals;

    MapRendererSettings settings;

    settings.width = render_settings.at("width"sv)->AsDouble();
    settings.height = render_settings.at("height"sv)->AsDouble();
    settings.padding = render_settings.at("padding"sv)->AsDouble();
    settings.line_width = render_settings.at("line_width"sv)->AsDouble();
    settings.stop_radius = render_settings.at("stop_radius"sv)->AsDouble();
    settings.bus_label_font_size = render_settings.at("bus_label_font_size"sv)->AsInt();
    settings.bus_label_offset = std::move(ParseOffset(render_settings.at("bus_label_offset"sv)->AsArray()));
    settings.stop_label_font_size = render_settings.at("stop_label_font_size"sv)->AsInt();
    settings.stop_label_offset = std::move(ParseOffset(render_settings.at("stop_label_offset"sv)->AsArray()));
    settings.underlayer_color = std::move(ParseColor(render_settings.at("underlayer_color"sv)));
    settings.underlayer_width = render_settings.at("underlayer_width"sv)->AsDouble();
    settings.color_palette = std::move(ParsePaletteColors(render_settings.at("color_palette"sv)->AsArray()));

    request_handler.RenderMap(std::move(settings));
}

} // namespace detail_base

// ----------------------------------------------------------------------------

namespace detail_stat {

void RequestStatStopProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Dict& request) {
    using namespace std::literals;
    using namespace transport_catalogue::stop_catalogue;

    std::string_view name = request.at("name"s).AsString();
    int id = request.at("id"s).AsInt();

    // TODO: �������� ��� ���������� ���������� �� ������, � �� �� ��������
    const auto [buses, stop_has_been_found] = request_handler.GetStopBuses(name);

    if (stop_has_been_found) {
        json::Array buses_arr;
        for (const std::string_view bus : buses) {
            buses_arr.push_back(std::string(bus));
        }

        builder
            .StartDict()
                .Key("buses"s).Value(std::move(buses_arr))
                .Key("request_id"s).Value(id)
            .EndDict();
    }
    else {
        builder
            .StartDict()
                .Key("error_message"s).Value("not found"s)
                .Key("request_id"s).Value(id)
            .EndDict();
    }
}

void RequestStatBusProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Dict& request) {
    using namespace std::literals;

    std::string_view name = request.at("name"s).AsString();
    int id = request.at("id"s).AsInt();

    // TODO: �������� ��� ���������� ���������� �� ������, � �� �� ��������
    const auto [it, bus_has_been_found] = request_handler.GetBus(name);

    const bus_catalogue::Bus* bus = (bus_has_been_found) ? (*it).second : nullptr;

    if (bus_has_been_found) {
        double curvature = (std::abs(bus->route_geo_length) > 1e-6) ? bus->route_true_length / bus->route_geo_length : 0.0;

        builder
            .StartDict()
                .Key("curvature"s).Value(curvature)
                .Key("request_id"s).Value(id)
                .Key("route_length"s).Value(bus->route_true_length)
                .Key("stop_count"s).Value(static_cast<int>(bus->stops_on_route))
                .Key("unique_stop_count"s).Value(static_cast<int>(bus->unique_stops))
            .EndDict();
    }
    else {
        builder
            .StartDict()
                .Key("error_message"s).Value("not found"s)
                .Key("request_id"s).Value(id)
            .EndDict();
    }
}

void RequestMapProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Dict& request) {
    using namespace std::literals;

    if (!request_handler.GetMap()) {
        throw std::logic_error("Map hasn't been rendered!"s);
    }

    int id = request.at("id"s).AsInt();

    builder
        .StartDict()
        .Key("map"s).Value(*request_handler.GetMap())
        .Key("request_id"s).Value(id)
        .EndDict();
}

void RequestStatProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Node* node) {
    using namespace std::literals;

    const json::Dict& request = node->AsMap();
    std::string_view type = request.at("type"s).AsString();

    if (type == "Stop"sv) {
        RequestStatStopProcess(builder, request_handler, request);
    }
    else if (type == "Bus"sv) {
        RequestStatBusProcess(builder, request_handler, request);
    }
    else if (type == "Map"sv) {
        RequestMapProcess(builder, request_handler, request);
    }
    else {
        throw json::ParsingError("Unknown type "s + std::string(type) + " in RequestStatProcess"s);
    }
}

} // namespace detail_stat

// ----------------------------------------------------------------------------

void RequestHandlerProcess(std::istream& input, std::ostream& output) {
    try {
        using namespace std::literals;

        TransportCatalogue catalogue;
        RequestHandler request_handler(catalogue);

        json::Builder builder;
        json::Reader reader(input);

        // ���������� ���������
        for (const json::Node* node : reader.StopRequests()) {
            detail_base::RequestBaseStopProcess(request_handler, node, reader.RoadDistances());
        }

        // ���������� �������� ���������� ����� �����������
        for (const auto& [name_from, distances] : reader.RoadDistances()) {
            for (const auto& [name_to, distance] : *distances) {
                request_handler.AddDistance(name_from, name_to, distance.AsDouble());
            }
        }

        // ���������� ���������� ��������
        for (const json::Node* node : reader.BusRequests()) {
            detail_base::RequestBaseBusProcess(request_handler, node);
        }

        // �������������� ����� ���������
        if (!reader.RenderSettings().empty()) {
            detail_base::RequestBaseMapProcess(request_handler, reader.RenderSettings());
        }

        // ������� ���������
        builder.StartArray();
        for (const json::Node* node : reader.StatRequests()) {
            detail_stat::RequestStatProcess(builder, request_handler, node);
        }
        builder.EndArray();

        json::Print(json::Document(builder.Build()), output);
    }
    catch (const json::ParsingError& error) {
        output << error.what();
    }
    catch (const std::exception& error) {
        output << std::string("Unknown error has occured") << std::endl;
        output << error.what();
    }
    catch (...) {
        output << std::string("Unknown error has occured") << std::endl;
    }
}

} // namespace request_handler
