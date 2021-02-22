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

// ----------------------------------------------------------------------------

namespace detail {

template <typename VirtualStruct>
using ConstIterator = typename std::unordered_map<std::string_view, const VirtualStruct*>::const_iterator;

template <typename VirtualStruct>
using VirtualPair = std::pair<ConstIterator<VirtualStruct>, bool>;

template <typename VirtualStruct>
class VirtualCatalogue {
public:
    VirtualCatalogue() = default;

    VirtualPair<VirtualStruct> At(const std::string_view& name) const {
        auto it = data_.find(name);
        return std::make_pair(it, (it == data_.end() ? false : true));
    }

    VirtualPair<VirtualStruct> Push(const std::string_view& name, const VirtualStruct* data) {
        return data_.emplace(name, data);
    }

    ConstIterator<VirtualStruct> begin() const {
        return data_.begin();
    }

    ConstIterator<VirtualStruct> end() const {
        return data_.end();
    }

private:
    std::unordered_map<std::string_view, const VirtualStruct*> data_;
};

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

private:
    std::deque<Stop> stops_ = {};
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

    Bus() = default;

    Bus(std::string&& name, std::deque<const stop_catalogue::Stop*>&& route, double route_geo_length, double route_true_length, RouteType route_type);
};

std::ostream& operator<< (std::ostream& out, const Bus& bus);

class Catalogue {
public:
    Catalogue() = default;

    const Bus* Push(std::string&& name, std::vector<std::string_view>&& string_route, RouteType type,
        const detail::VirtualCatalogue<stop_catalogue::Stop>& stops_catalogue, const stop_catalogue::DistancesContainer& stops_distances);

private:
    double CalcRouteGeolength(const std::deque<const stop_catalogue::Stop*>& route, RouteType route_type);
    double CalcRouteTruelength(const std::deque<const stop_catalogue::Stop*>& route, const stop_catalogue::DistancesContainer& stops_distances, RouteType route_type);

private:
    std::deque<Bus> buses_ = {};
};

} // namespace bus_catalogue

} // namespace transport_catalogue
