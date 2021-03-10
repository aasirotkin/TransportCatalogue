#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddBus(bus_catalogue::Bus&& add_bus) {
    const bus_catalogue::Bus* bus = buses_.Push(std::move(add_bus));
    virtual_buses_.Push(bus->name, bus);

    for (const stop_catalogue::Stop* stop : bus->route) {
        stops_.PushBusToStop(stop, bus->name);
    }
}

void TransportCatalogue::AddStop(std::string&& name, std::string&& string_coord) {
    const stop_catalogue::Stop* stop = stops_.Push(std::move(name), std::move(string_coord));
    virtual_stops_.Push(stop->name, stop);
}

void TransportCatalogue::AddStop(std::string&& name, Coordinates&& coord) {
    const stop_catalogue::Stop* stop = stops_.Push(std::move(name), std::move(coord));
    virtual_stops_.Push(stop->name, stop);
}

void TransportCatalogue::AddDistanceBetweenStops(const std::string_view& stop_from_name, const std::string_view& stop_to_name, double distance) {
    auto [stop_from_it, fist_stop_found] = virtual_stops_.At(stop_from_name);
    auto [stop_to_it, second_stop_found] = virtual_stops_.At(stop_to_name);
    stops_.AddDistance(stop_from_it->second, stop_to_it->second, distance);
}

detail::VirtualPair<bus_catalogue::Bus> TransportCatalogue::GetBus(const std::string_view& name) const {
    return virtual_buses_.At(name);
}

const stop_catalogue::BusesToStopNames& TransportCatalogue::GetBusesForStop(const std::string_view& name) const {
    static const std::set<std::string_view> empty_set = {};
    auto [it, stop_has_been_found] = virtual_stops_.At(name);
    return (stop_has_been_found)
        ? stops_.GetBuses(it->second)
        : empty_set;
}

const detail::VirtualCatalogue<stop_catalogue::Stop>& TransportCatalogue::GetStops() const {
    return virtual_stops_;
}

const detail::VirtualCatalogue<bus_catalogue::Bus>& TransportCatalogue::GetBuses() const {
    return virtual_buses_;
}

const stop_catalogue::Catalogue& TransportCatalogue::GetStopsCatalogue() const {
    return stops_;
}

const bus_catalogue::Catalogue& TransportCatalogue::GetBusCatalogue() const {
    return buses_;
}

void TransportCatalogue::SetBusRouteCommonSettings(RouteSettings&& settings) {
    buses_.SetRouteSettings(std::move(settings));
}

}
