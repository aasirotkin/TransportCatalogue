#pragma once

#include "domain.h"

#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace transport_catalogue {

// ----------------------------------------------------------------------------

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddBus(std::string&& name, std::vector<std::string_view>&& route, RouteType type);

    void AddStop(std::string&& name, std::string&& string_coord);

    void AddDistanceBetweenStops(const std::string_view& stop_from_name, const std::string_view& stop_to_name, double distance);

    detail::VirtualPair<bus_catalogue::Bus> GetBus(const std::string_view& name) const;

    std::pair<std::set<std::string_view>, bool> GetBusesForStop(const std::string_view& name) const;

private:
    bus_catalogue::Catalogue buses_;
    detail::VirtualCatalogue<bus_catalogue::Bus> virtual_buses_;

    stop_catalogue::Catalogue stops_;
    detail::VirtualCatalogue<stop_catalogue::Stop> virtual_stops_;
};

// ----------------------------------------------------------------------------

} // namespace transport_catalogue
