#pragma once

#include "domain.h"
#include "router.h"

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

    void AddBus(bus_catalogue::Bus&& add_bus);

    void AddStop(std::string&& name, std::string&& string_coord);

    void AddStop(std::string&& name, Coordinates&& coord);

    void AddDistanceBetweenStops(const std::string_view& stop_from_name, const std::string_view& stop_to_name, double distance);

    detail::VirtualPair<bus_catalogue::Bus> GetBus(const std::string_view& name) const;

    const stop_catalogue::BusesToStopNames& GetBusesForStop(const std::string_view& name) const;

    const detail::VirtualCatalogue<stop_catalogue::Stop>& GetStops() const;

    const detail::VirtualCatalogue<bus_catalogue::Bus>& GetBuses() const;

    const stop_catalogue::Catalogue& GetStopsCatalogue() const;

    const bus_catalogue::Catalogue& GetBusCatalogue() const;

    void SetBusRouteCommonSettings(RouteSettings&& settings);

private:
    bus_catalogue::Catalogue buses_;
    detail::VirtualCatalogue<bus_catalogue::Bus> virtual_buses_;

    stop_catalogue::Catalogue stops_;
    detail::VirtualCatalogue<stop_catalogue::Stop> virtual_stops_;
};

// ----------------------------------------------------------------------------

} // namespace transport_catalogue
