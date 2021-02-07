#include "request_handler.h"

#include "geo.h"
#include "json_reader.h"
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

void RequestStatStopProcess(TransportCatalogue& catalogue, const json::Dict& request, std::ostream& output) {
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

void RequestStatBusProcess(TransportCatalogue& catalogue, const json::Dict& request, std::ostream& output) {
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

void RequestStatProcess(TransportCatalogue& catalogue, const json::Array& stat_requests, std::ostream& output) {
    using namespace std::literals;

    bool first = true;

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
        const json::Array& stat_requests = input_requests.at("stat_requests"s).AsArray();

        TransportCatalogue catalogue;

        RequestBaseProcess(catalogue, base_requests);
        RequestStatProcess(catalogue, stat_requests, output);
    }
    catch (std::exception& ex) {
        output << ex.what() << std::endl;
    }
}

} // namespace request_handler
