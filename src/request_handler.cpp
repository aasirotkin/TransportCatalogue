#include "request_handler.h"

#include "geo.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace request_handler {

using namespace transport_catalogue;

void RequestBaseStopProcess(
    TransportCatalogue& catalogue,
    const std::vector<const json::Node*>& stop_requests,
    const std::unordered_map<std::string_view, const json::Dict*>& road_distances) {
    using namespace std::literals;

    for (const json::Node* node_request : stop_requests) {
        const json::Dict& request = (*node_request).AsMap();

        std::string name = request.at("name"s).AsString();

        catalogue.AddStop(
            std::move(name),
            Coordinates{ request.at("latitude"s).AsDouble(), request.at("longitude"s).AsDouble() });
    }

    for (const auto& [name_from, distances] : road_distances) {
        for (const auto& [name_to, distance] : *distances) {
            catalogue.AddDistanceBetweenStops(name_from, name_to, distance.AsDouble());
        }
    }
}

void RequestBaseBusProcess(
    TransportCatalogue& catalogue,
    const std::vector<const json::Node*>& bus_requests) {
    using namespace std::literals;

    for (const json::Node* node_request : bus_requests) {
        const json::Dict& request = (*node_request).AsMap();

        std::string name = request.at("name"s).AsString();

        transport_catalogue::RouteType type = (request.at("is_roundtrip"s).AsBool())
            ? transport_catalogue::RouteType::Round
            : transport_catalogue::RouteType::BackAndForth;

        std::vector<std::string_view> route;
        for (const json::Node& node_stops : request.at("stops").AsArray()) {
            route.push_back(node_stops.AsString());
        }

        catalogue.AddBus(std::string(name), std::move(route), type);
    }
}

void RequestBaseProcess(TransportCatalogue& catalogue, const json::Array& base_requests) {
    using namespace std::literals;

    std::vector<const json::Node*> stop_requests;
    std::vector<const json::Node*> bus_requests;
    std::unordered_map<std::string_view, const json::Dict*> road_distances;

    for (const json::Node& node_request : base_requests) {
        const json::Dict& request = node_request.AsMap();
        std::string_view type = request.at("type"s).AsString();
        if (type == "Stop"sv) {
            stop_requests.push_back(&node_request);
            road_distances.insert({
                request.at("name"s).AsString(),
                &request.at("road_distances"s).AsMap()
                });
        }
        else if (type == "Bus"sv) {
            bus_requests.push_back(&node_request);
        }
        else {
            throw json::ParsingError("Unknown type \""s + std::string(type) + "\""s);
        }
    }

    RequestBaseStopProcess(catalogue, stop_requests, road_distances);
    RequestBaseBusProcess(catalogue, bus_requests);
}

void RequestStatStopProcess(const TransportCatalogue& catalogue, const json::Dict& request, std::ostream& output) {
    using namespace std::literals;
    using namespace transport_catalogue::stop_catalogue;

    std::string_view name = request.at("name"s).AsString();
    int id = request.at("id"s).AsInt();

    const auto& [buses, stop_has_been_found] = catalogue.GetBusesForStop(name);

    if (stop_has_been_found) {
        output << "{\n"sv;
        output << "\t\"buses\": [\n"sv;
        output << "\t\t"sv << buses << "\n"sv;
        output << "\t],\n"sv;
        output << "\t\"request_id\": "sv << id << "\n"sv;
        output << "}"sv;
    }
    else {
        output << "{\n"sv;
        output << "\t\"request_id\": "sv << id << ",\n"sv;
        output << "\t\"error_message\": \"not found\"\n"sv;
        output << "}"sv;
    }
}

void RequestStatBusProcess(const TransportCatalogue& catalogue, const json::Dict& request, std::ostream& output) {
    using namespace std::literals;

    std::string_view name = request.at("name"s).AsString();
    int id = request.at("id"s).AsInt();

    auto [it, bus_has_been_found] = catalogue.GetBus(name);

    if (bus_has_been_found) {
        const bus_catalogue::Bus* bus = (*it).second;

        double curvature = (std::abs(bus->route_geo_lenght) > 1e-6) ? bus->route_true_lenght / bus->route_geo_lenght : 0.0;

        output << "{\n"sv;
        output << "\t\"curvature\": "sv << curvature << ",\n"sv;
        output << "\t\"request_id\": "sv << id << ",\n"sv;
        output << "\t\"route_length\": "sv << bus->route_true_lenght << ",\n"sv;
        output << "\t\"stop_count\": "sv << bus->stops_on_route << ",\n"sv;
        output << "\t\"unique_stop_count\": "sv << bus->unique_stops << "\n"sv;
        output << "}"sv;
    }
    else {
        output << "{\n"sv;
        output << "\t\"request_id\": "sv << id << ",\n"sv;
        output << "\t\"error_message\": \"not found\"\n"sv;
        output << "}"sv;
    }
}

svg::Color ParseColor(const json::Node& node) {
    if (node.IsString()) {
        return svg::Color(node.AsString());
    }
    else if (node.IsArray()) {
        const json::Array& array_color = node.AsArray();
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
    return svg::Color{};
}

std::vector<svg::Color> ParsePaletteColors(const json::Array& array_color_palette) {
    std::vector<svg::Color> color_palette;
    for (const json::Node& node : array_color_palette) {
        color_palette.push_back(std::move(ParseColor(node)));
    }
    return color_palette;
}

svg::Point ParseOffset(const json::Array& offset) {
    return svg::Point{ offset.at(0).AsDouble(), offset.at(1).AsDouble() };
}

MapRendererSettings CreateMapRendererSettings(const json::Dict& render_settings) {
    using namespace std::string_literals;

    MapRendererSettings settings;

    settings.width = render_settings.at("width"s).AsDouble();
    settings.height = render_settings.at("height"s).AsDouble();
    settings.padding = render_settings.at("padding"s).AsDouble();
    settings.line_width = render_settings.at("line_width"s).AsDouble();
    settings.stop_radius = render_settings.at("stop_radius"s).AsDouble();
    settings.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();
    settings.bus_label_offset = std::move(ParseOffset(render_settings.at("bus_label_offset"s).AsArray()));
    settings.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();
    settings.stop_label_offset = std::move(ParseOffset(render_settings.at("stop_label_offset"s).AsArray()));
    settings.underlayer_color = std::move(ParseColor(render_settings.at("underlayer_color"s)));
    settings.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();
    settings.color_palette = std::move(ParsePaletteColors(render_settings.at("color_palette"s).AsArray()));

    return settings;
}

void RequestRenderProcess(std::string_view map_render_result, const json::Dict& request, std::ostream& output) {
    using namespace std::literals;

    //output << map_render_result;
    //throw json::ParsingError(""s);

    int id = request.at("id"s).AsInt();

    output << "{\n"sv;
    output << "\t\"map\": "sv;
    json::Print(map_render_result, output);
    output << ",\n\t\"request_id\": "sv << id << "\n"sv;
    output << "}"sv;
}

std::string RequestMap(const TransportCatalogue& catalogue, const json::Dict& render_settings) {
    MapRenderer render(
        std::move(CreateMapRendererSettings(render_settings)),
        catalogue.GetStopsCatalogue(),
        catalogue.GetStops(),
        catalogue.GetBuses());

    std::ostringstream oss;
    render.Render(oss);

    return oss.str();
}

void RequestStatProcess(const TransportCatalogue& catalogue, const json::Array& stat_requests, const json::Dict& render_settings, std::ostream& output) {
    using namespace std::literals;

    bool first = true;

    std::optional<std::string> map_render_result;

    output << "[\n\n"sv;

    for (const json::Node& node_request : stat_requests) {
        const json::Dict& request = node_request.AsMap();
        std::string_view type = request.at("type"s).AsString();

        if (!first) {
            output << ",\n"sv;
        }

        if (type == "Stop"sv) {
            RequestStatStopProcess(catalogue, request, output);
        }
        else if (type == "Bus"sv) {
            RequestStatBusProcess(catalogue, request, output);
        }
        else if (type == "Map"sv) {
            if (!map_render_result) {
                map_render_result = RequestMap(catalogue, render_settings);
            }
            RequestRenderProcess(*map_render_result, request, output);
        }
        else {
            throw json::ParsingError("Unknown type "s + std::string(type) + " in RequestStatProcess"s);
        }

        first = false;
    }

    output << "\n\n]\n"sv;
}

void RequestHandler(std::istream& input, std::ostream& output) {
    try {
        using namespace std::literals;

        json::Document doc = std::move(json::Load(input));

        const json::Dict& input_requests = doc.GetRoot().AsMap();

        const json::Array& base_requests = input_requests.at("base_requests"s).AsArray();

        static const json::Dict empty_dict{};
        bool is_render_settings = (input_requests.count("render_settings"s) > 0);
        const json::Dict& render_settings = (is_render_settings) ? input_requests.at("render_settings"s).AsMap() : empty_dict;
        const json::Array& stat_requests = input_requests.at("stat_requests"s).AsArray();

        TransportCatalogue catalogue;

        RequestBaseProcess(catalogue, base_requests);
        RequestStatProcess(catalogue, stat_requests, render_settings, output);
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
