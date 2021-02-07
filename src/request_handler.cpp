#include "request_handler.h"

#include "geo.h"
#include "json_reader.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace request_handler {

using namespace transport_catalogue;

namespace parse_detail {

std::string ReadLine(std::istream& input) {
    std::string s;
    std::getline(input, s);
    return s;
}

int ReadLineWithNumber(std::istream& input) {
    int number = 0;
    input >> number;
    ReadLine(input);
    return number;
}

void TrimSpacesInPlace(std::string_view& value) {
    value.remove_prefix(value.find_first_not_of(' '));
    value.remove_suffix(value.size() - value.find_last_not_of(' ') - 1);
}

std::pair<std::string_view, std::string_view> ParseStringOnNameData(const std::string_view& query) {
    size_t start_name_pos = query.find_first_of(' ') + 1;
    size_t end_name_pos = query.find_first_of(':', start_name_pos);
    return {
        query.substr(start_name_pos, end_name_pos - start_name_pos),
        query.substr(end_name_pos + 1) };
}

std::vector<std::string_view> SplitIntoWords(std::string_view text, char sep = ' ')
{
    std::vector<std::string_view> result;
    int64_t pos_end = std::string_view::npos;
    while (true) {
        int64_t space = text.find(sep);
        result.push_back(text.substr(0, space));
        if (space == pos_end) {
            break;
        }
        text.remove_prefix(space + 1);
    }

    for (std::string_view& result_i : result) {
        TrimSpacesInPlace(result_i);
    }

    return result;
}

} // namespace parse_detail

namespace detail {

StopRequest::StopRequest(const std::string_view& request) {
    auto [parse_name, data] = parse_detail::ParseStringOnNameData(request);

    size_t data_size = data.size();
    size_t first_comma = data.find_first_of(',');
    size_t second_comma = std::min(data.find_first_of(',', first_comma + 1), data_size);

    name = parse_name;
    coord = data.substr(0, second_comma);
    parse_detail::TrimSpacesInPlace(coord);
    if (second_comma != data_size) {
        dist = parse_detail::SplitIntoWords(data.substr(second_comma + 1, data.size()), ',');
    }
}

} // namespace detail

namespace input_detail {

void InputStop(TransportCatalogue& transport_catalogue, const std::string_view& name, const std::string_view& coord) {
    transport_catalogue.AddStop(std::string(name), std::string(coord));
}

void InputStopDistance(TransportCatalogue& transport_catalogue, const detail::StopRequest& request) {
    for (const std::string_view& dist_request : request.dist) {
        size_t m_pos = dist_request.find_first_of('m');
        size_t name_start_pos = m_pos + 5;

        double dist = std::stod(std::string(dist_request.substr(0, m_pos)));

        transport_catalogue.AddDistanceBetweenStops(request.name, dist_request.substr(name_start_pos), dist);
    }
}

void InputBus(TransportCatalogue& transport_catalogue, const std::string_view& string_bus) {
    auto [name, data] = parse_detail::ParseStringOnNameData(string_bus);

    RouteType type = (data.find('>') != std::string_view::npos) ? RouteType::Round : RouteType::BackAndForth;
    char sep = (type == RouteType::Round) ? '>' : '-';

    std::vector<std::string_view> route = parse_detail::SplitIntoWords(data, sep);

    transport_catalogue.AddBus(std::string(name), std::move(route), type);
}

void InputQueries(TransportCatalogue& transport_catalogue, const std::vector<std::string>& queries) {
    std::vector<detail::StopRequest> stop_request;
    std::vector<std::string_view> bus_request;
    for (const std::string_view& request : queries) {
        if (request.rfind(std::string("Stop"), 0) == 0) {
            stop_request.push_back(detail::StopRequest(request));
        }
        else {
            bus_request.push_back(request);
        }
    }

    for (const detail::StopRequest& request : stop_request) {
        InputStop(transport_catalogue, request.name, request.coord);
    }

    for (const detail::StopRequest& request : stop_request) {
        InputStopDistance(transport_catalogue, request);
    }

    for (const std::string_view& request : bus_request) {
        InputBus(transport_catalogue, request);
    }
}

} // namespace input_detail

namespace output_detail {

void BusOutputRequest(const TransportCatalogue& transport_catalogue, std::string&& bus_name, std::ostream& out) {
    auto [it, bus_has_been_found] = transport_catalogue.GetBus(bus_name);

    if (bus_has_been_found) {
        out << *(*it).second;
    }
    else {
        out << std::string("Bus ") << bus_name << std::string(": not found");
    }
    out << "\n";
}

void StopOutputRequest(const TransportCatalogue& transport_catalogue, std::string&& stop_name, std::ostream& out) {
    using namespace transport_catalogue::stop_catalogue;

    auto [buses, stop_has_been_found] = transport_catalogue.GetBusesForStop(stop_name);

    if (stop_has_been_found) {
        if (!buses.empty()) {
            out << std::string("Stop ") << stop_name << std::string(": buses ") << buses;
        }
        else {
            out << std::string("Stop ") << stop_name << std::string(": no buses");
        }
    }
    else {
        out << std::string("Stop ") << stop_name << std::string(": not found");
    }
    out << "\n";
}

void OutputRequest(const TransportCatalogue& transport_catalogue, const std::string& request, std::ostream& out) {
    size_t space = request.find_first_of(' ');
    std::string request_type = request.substr(0, space);
    std::string name = request.substr(space + 1);

    static const std::string bus_request = std::string("Bus");
    static const std::string stop_request = std::string("Stop");

    if (request_type == bus_request) {
        BusOutputRequest(transport_catalogue, std::move(name), out);
    }
    else if (request_type == stop_request) {
        StopOutputRequest(transport_catalogue, std::move(name), out);
    }
}

} // namespace output_detail

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
    std::string id = std::to_string(request.at("id"s).AsInt());

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
    std::string id = std::to_string(request.at("id"s).AsInt());

    auto [it, bus_has_been_found] = catalogue.GetBus(name);

    if (bus_has_been_found) {
        const bus_catalogue::Bus* bus = (*it).second;

        double curvature = bus->route_true_lenght / bus->route_geo_lenght;

        output << "{\n"sv;
        output << "\t\"curvature\": "sv << curvature << ",\n"sv;
        output << "\t\"request_id\": "sv << id << ",\n"sv;
        output << "\t\"route_length\": "sv << bus->route_true_lenght << ",\n"sv;
        output << "\t\"stop_count\": "sv << bus->stops_on_route << ",\n"sv;
        output << "\t\"unique_stop_count\": "sv << bus->unique_stops << ",\n"sv;
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
    for (const json::Node& node_request : stat_requests) {
        const json::Dict& request = node_request.AsMap();
        std::string_view type = request.at("type"s).AsString();

        if (!first) {
            output << ",\n"sv;
        }

        if (type == "Stop"sv) {
            RequestStatStopProcess(catalogue, request, output);
        }
        else if (type == "Bus"s) {
            RequestStatBusProcess(catalogue, request, output);
        }
        else {
            throw json::ParsingError("Unknown type "s + std::string(type) + " in RequestStatProcess"s);
        }

        first = false;
    }
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
