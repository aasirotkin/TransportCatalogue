#include "transport_catalogue.h"

#include <iomanip>
#include <numeric>
#include <utility>

// ----------------------------------------------------------------------------

Bus::Bus(std::string&& name, std::deque<const Stop*>&& rout, RoutType rout_type)
    : name(name)
    , rout(rout)
    , rout_type(rout_type)
    , stops_on_rout(rout.size())
    , unique_stops(stops_on_rout) {
    std::vector<double> distance(rout.size() - 1);
    std::transform(
        rout.begin(), rout.end() - 1,
        rout.begin() + 1, distance.begin(),
        [](const Stop* from, const Stop* to) {
            return ComputeDistance(from->coord, to->coord);
        });
    rout_lenght = std::reduce(distance.begin(), distance.end());
    if (rout_type == RoutType::BackAndForth) {
        rout_lenght *= 2.0;
        stops_on_rout = stops_on_rout * 2 - 1;
    }
    else if (rout_type == RoutType::Round) {
        unique_stops -= 1;
    }
}

std::ostream& operator<<(std::ostream& out, const Bus& bus) {
    static const char* str_bus = "Bus ";
    static const char* str_sep = ": ";
    static const char* str_comma = ", ";
    static const char* str_space = " ";
    static const char* str_stops_on_rout = "stops on route";
    static const char* str_unique_stops = "unique stops";
    static const char* str_route_lenght = "route length";

    out << str_bus << bus.name << str_sep;
    out << bus.stops_on_rout << str_space << str_stops_on_rout << str_comma;
    out << bus.unique_stops << str_space << str_unique_stops << str_comma;
    out << std::setprecision(6) << bus.rout_lenght << str_space << str_route_lenght;

    return out;
}

// ----------------------------------------------------------------------------

const Stop* StopCatalogue::push(std::string&& name, std::string&& string_coord) {
    push_back({ std::move(name), Coordinates::ParseFromStringView(string_coord) });
    return &back();
}

// ----------------------------------------------------------------------------

const Bus* BusCatalogue::push(std::string&& name, std::vector<std::string_view>&& string_rout, RoutType type, const VirtualStopCatalogue& stops_catalogue) {
    std::deque<const Stop*> stops;
    for (const std::string_view& stop_name : string_rout) {
        auto [it, res] = stops_catalogue.at(stop_name);
        if (res) {
            stops.push_back((*it).second);
        }
    }
    push_back(Bus(std::move(name), std::move(stops), type));
    return &back();
}

// ----------------------------------------------------------------------------
