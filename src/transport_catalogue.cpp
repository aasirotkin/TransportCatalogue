#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddBus(std::string&& name, std::vector<std::string_view>&& route, RouteType type) {
    const bus_catalogue::Bus* bus = buses_.Push(std::move(name), std::move(route), type, virtual_stops_, stops_.GetDistances());
    virtual_buses_.Push(bus->name, bus);

    for (const stop_catalogue::Stop* stop : bus->route) {
        stops_.PushBusToStop(stop, bus->name);
    }
}

void TransportCatalogue::AddStop(std::string&& name, std::string&& string_coord) {
    const stop_catalogue::Stop* stop = stops_.Push(std::move(name), std::move(string_coord));
    virtual_stops_.Push(stop->name, stop);
}

void TransportCatalogue::AddDistanceBetweenStops(const std::string_view& stop_from_name, const std::string_view& stop_to_name, double distance) {
    auto [stop_from_it, fist_stop_found] = virtual_stops_.At(stop_from_name);
    auto [stop_to_it, second_stop_found] = virtual_stops_.At(stop_to_name);
    stops_.AddDistance((*stop_from_it).second, (*stop_to_it).second, distance);
}

detail::VirtualPair<bus_catalogue::Bus> TransportCatalogue::GetBus(const std::string_view& name) const {
    return virtual_buses_.At(name);
}

std::pair<std::set<std::string_view>, bool> TransportCatalogue::GetBusesForStop(const std::string_view& name) const {
    static const std::set<std::string_view> empty_set = {};
    auto [it, stop_has_been_found] = virtual_stops_.At(name);
    return (stop_has_been_found)
        ? std::make_pair(stops_.GetBuses((*it).second), stop_has_been_found)
        : std::make_pair(empty_set, stop_has_been_found);
}

}
