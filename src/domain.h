#pragma once

#include "geo.h"

#include <deque>
#include <functional>
#include <set>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace transport_catalogue {

// ----------------------------------------------------------------------------

enum class RouteType {
    Direct,
    BackAndForth,
    Round
};

struct RouteSettings {
    double bus_velocity = 0.0;
    int bus_wait_time = 0;
};

// ----------------------------------------------------------------------------

namespace detail {

template <typename Pointer>
using PointerPair = std::pair<const Pointer*, const Pointer*>;

template <typename Pointer>
class PointerPairHasher {
public:
    size_t operator() (const PointerPair<Pointer>& stop_pair) const {
        return hasher_(stop_pair.first) * 47 + hasher_(stop_pair.second);
    }
private:
    std::hash<const Pointer*> hasher_;
};

} // namespace detail

// ----------------------------------------------------------------------------

namespace stop_catalogue {

struct Stop {
    std::string name;
    Coordinates coord;

    bool operator== (const Stop& other) const {
        return name == other.name;
    }

    bool operator!= (const Stop& other) const {
        return !(*this == other);
    }
};

using BusesToStopNames = std::set<std::string_view>;
using DistancesContainer = std::unordered_map<detail::PointerPair<Stop>, double, detail::PointerPairHasher<Stop>>;

std::ostream& operator<< (std::ostream& out, const BusesToStopNames& buses);

class Catalogue {
public:
    Catalogue() = default;

    const Stop* Push(std::string&& name, std::string&& string_coord);

    const Stop* Push(std::string&& name, Coordinates&& coord);

    void PushBusToStop(const Stop* stop, const std::string_view& bus_name);

    void AddDistance(const Stop* stop_1, const Stop* stop_2, double distance);

    const BusesToStopNames& GetBuses(const Stop* stop) const {
        return stop_buses_.at(stop);
    }

    const DistancesContainer& GetDistances() const {
        return distances_between_stops_;
    }

    bool IsEmpty(const Stop* stop) const {
        return stop_buses_.count(stop) == 0 || stop_buses_.at(stop).empty();
    }

    size_t Size() const {
        return stops_.size();
    }

    std::optional<const Stop*> At(std::string_view name) const {
        if (name_to_stop_.count(name) > 0) {
            return name_to_stop_.at(name);
        }
        return std::nullopt;
    }

    auto begin() const {
        return name_to_stop_.begin();
    }

    auto end() const {
        return name_to_stop_.end();
    }

private:
    std::deque<Stop> stops_ = {};
    std::unordered_map<std::string_view, const Stop*> name_to_stop_ = {};
    std::unordered_map<const Stop*, BusesToStopNames> stop_buses_ = {};
    DistancesContainer distances_between_stops_ = {};
};

} // namespace stop_catalogue

// ----------------------------------------------------------------------------

namespace bus_catalogue {

struct Bus {
    std::string name = {};
    std::deque<const stop_catalogue::Stop*> route = {};
    RouteType route_type = RouteType::Direct;
    double route_geo_length = 0.0;
    double route_true_length = 0.0;
    size_t stops_on_route = 0;
    size_t unique_stops = 0;
    RouteSettings route_settings = {};

    Bus() = default;
};

class BusHelper {
public:
    BusHelper& SetName(std::string&& name) {
        name_ = std::move(name);
        return *this;
    }

    BusHelper& SetRouteType(RouteType route_type) {
        route_type_ = route_type;
        return *this;
    }

    BusHelper& SetStopNames(std::vector<std::string_view>&& stop_names) {
        stop_names_ = std::move(stop_names);
        return *this;
    }

    BusHelper& SetRouteSettings(RouteSettings settings) {
        settings_ = std::move(settings);
        return *this;
    }

    Bus Build(const stop_catalogue::Catalogue& stops_catalogue);

private:
    double CalcRouteGeoLength(const std::deque<const stop_catalogue::Stop*>& route, RouteType route_type) const;
    double CalcRouteTrueLength(const std::deque<const stop_catalogue::Stop*>& route, const stop_catalogue::DistancesContainer& stops_distances, RouteType route_type) const;

private:
    std::string name_;
    RouteType route_type_;
    std::vector<std::string_view> stop_names_;
    RouteSettings settings_;
};

std::ostream& operator<< (std::ostream& out, const Bus& bus);

class Catalogue {
public:
    Catalogue() = default;

    const Bus* Push(Bus&& bus);

    void SetRouteSettings(RouteSettings&& settings) {
        settings_ = std::move(settings);
    }

    const RouteSettings& GetRouteSettings() const {
        return settings_;
    }

    std::optional<const Bus*> At(std::string_view name) const {
        if (name_to_bus_.count(name) > 0) {
            return name_to_bus_.at(name);
        }
        return std::nullopt;
    }

    auto begin() const {
        return name_to_bus_.begin();
    }

    auto end() const {
        return name_to_bus_.end();
    }

    size_t Size() const {
        return buses_.size();
    }

private:
    std::deque<Bus> buses_ = {};
    std::unordered_map<std::string_view, const Bus*> name_to_bus_ = {};
    RouteSettings settings_ = {};
};

} // namespace bus_catalogue

} // namespace transport_catalogue
