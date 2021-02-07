#include "request_handler.h"

#include "geo.h"

#include <algorithm>

using namespace transport_catalogue;

// ---------- Input queries ---------------------------------------------------

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

std::pair<std::string_view, std::string_view> ParseStringOnNameData(const std::string_view& query) {
    size_t start_name_pos = query.find_first_of(' ') + 1;
    size_t end_name_pos = query.find_first_of(':', start_name_pos);
    return {
        query.substr(start_name_pos, end_name_pos - start_name_pos),
        query.substr(end_name_pos + 1) };
}

StopQuery::StopQuery(const std::string_view& query) {
    auto [parse_name, data] = ParseStringOnNameData(query);

    size_t data_size = data.size();
    size_t first_comma = data.find_first_of(',');
    size_t second_comma = std::min(data.find_first_of(',', first_comma + 1), data_size);

    name = parse_name;
    coord = data.substr(0, second_comma);
    TrimSpacesInPlace(coord);
    if (second_comma != data_size) {
        dist = SplitIntoWords(data.substr(second_comma + 1, data.size()), ',');
    }
}

void InputStop(TransportCatalogue& transport_catalogue, const std::string_view& name, const std::string_view& coord) {
    transport_catalogue.AddStop(std::string(name), std::string(coord));
}

void InputStopDistance(TransportCatalogue& transport_catalogue, const StopQuery& query) {
    for (const std::string_view& dist_query : query.dist) {
        size_t m_pos = dist_query.find_first_of('m');
        size_t name_start_pos = m_pos + 5;

        double dist = std::stod(std::string(dist_query.substr(0, m_pos)));

        transport_catalogue.AddDistanceBetweenStops(query.name, dist_query.substr(name_start_pos), dist);
    }
}

void InputBus(TransportCatalogue& transport_catalogue, const std::string_view& string_bus) {
    auto [name, data] = ParseStringOnNameData(string_bus);

    RouteType type = (data.find('>') != std::string_view::npos) ? RouteType::Round : RouteType::BackAndForth;
    char sep = (type == RouteType::Round) ? '>' : '-';

    std::vector<std::string_view> route = SplitIntoWords(data, sep);

    transport_catalogue.AddBus(std::string(name), std::move(route), type);
}

void InputQueries(TransportCatalogue& transport_catalogue, const std::vector<std::string>& queries) {
    std::vector<StopQuery> stop_query;
    std::vector<std::string_view> bus_query;
    for (const std::string_view& query : queries) {
        if (query.rfind(std::string("Stop"), 0) == 0) {
            stop_query.push_back(StopQuery(query));
        }
        else {
            bus_query.push_back(query);
        }
    }

    for (const StopQuery& query : stop_query) {
        InputStop(transport_catalogue, query.name, query.coord);
    }

    for (const StopQuery& query : stop_query) {
        InputStopDistance(transport_catalogue, query);
    }

    for (const std::string_view& query : bus_query) {
        InputBus(transport_catalogue, query);
    }
}

// ---------- Output queries --------------------------------------------------

void BusOutputQuery(const TransportCatalogue& transport_catalogue, std::string&& bus_name, std::ostream& out) {
    auto [it, bus_has_been_found] = transport_catalogue.GetBus(bus_name);

    if (bus_has_been_found) {
        out << *(*it).second;
    }
    else {
        out << std::string("Bus ") << bus_name << std::string(": not found");
    }
    out << "\n";
}

void StopOutputQuery(const TransportCatalogue& transport_catalogue, std::string&& stop_name, std::ostream& out) {
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

void OutputQuery(const TransportCatalogue& transport_catalogue, const std::string& query, std::ostream& out) {
    size_t space = query.find_first_of(' ');
    std::string query_type = query.substr(0, space);
    std::string name = query.substr(space + 1);

    static const std::string bus_query = std::string("Bus");
    static const std::string stop_query = std::string("Stop");

    if (query_type == bus_query) {
        BusOutputQuery(transport_catalogue, std::move(name), out);
    }
    else if (query_type == stop_query) {
        StopOutputQuery(transport_catalogue, std::move(name), out);
    }
}