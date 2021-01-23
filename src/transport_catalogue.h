#pragma once

#include "geo.h"

#include <algorithm>
#include <deque>
#include <functional>
#include <iostream>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <vector>

struct Stop {
    std::string name;
    Coordinates coord;

    bool operator== (const Stop* other) const {
        return name == other->name;
    }
};

enum class RoutType {
    Direct,
    BackAndForth,
    Round
};

// ----------------------------------------------------------------------------

struct Bus {
    std::string name = {};
    std::deque<const Stop*> rout = {};
    RoutType rout_type = RoutType::Direct;
    double rout_lenght = 0.0;
    double rout_true_lenght = 0.0;
    size_t stops_on_rout = 0;
    size_t unique_stops = 0;

    Bus() = default;

    Bus(std::string&& name, std::deque<const Stop*>&& rout, double rout_lenght, double rout_true_lenght, RoutType rout_type);
};

std::ostream& operator<< (std::ostream& out, const Bus& bus);

// ----------------------------------------------------------------------------

template <typename VirtualStruct>
class VirtualCatalogue {
    using ConstIterator = typename std::unordered_map<std::string_view, const VirtualStruct*>::const_iterator;
    using VirtualPair = std::pair<ConstIterator, bool>;
public:
    VirtualCatalogue() = default;

    VirtualPair At(const std::string_view& name) const {
        auto it = data_.find(name);
        return std::make_pair(it, (it == data_.end() ? false : true));
    }

    VirtualPair Push(const std::string_view& name, const VirtualStruct* data) {
        return data_.emplace(name, data);
    }
private:
    std::unordered_map<std::string_view, const VirtualStruct*> data_;
};

using VirtualStopCatalogue = VirtualCatalogue<Stop>;
using VirtualBusCatalogue = VirtualCatalogue<Bus>;

// ----------------------------------------------------------------------------

using StopsPointerPair = std::pair<const Stop*, const Stop*>;

class StopsPointerPairHasher {
public:
    size_t operator() (const StopsPointerPair& stop_pair) const {
        return hasher_(stop_pair.first) * 47 + hasher_(stop_pair.second);
    }
private:
    std::hash<const Stop*> hasher_;
};

// ----------------------------------------------------------------------------

using BusesToStopNames = std::set<std::string_view>;
using StopDistancesContainer = std::unordered_map<StopsPointerPair, double, StopsPointerPairHasher>;

std::ostream& operator<< (std::ostream& out, const BusesToStopNames& buses);

class StopCatalogue {
public:
    StopCatalogue() = default;

    const Stop* Push(std::string&& name, std::string&& string_coord);

    void PushBusToStop(const Stop* stop, const std::string_view& bus_name);

    void AddDistance(const Stop* stop_1, const Stop* stop_2, double distance);

    const std::set<std::string_view>& GetBuses(const Stop* stop) const {
        return stop_buses_.at(stop);
    }

    const StopDistancesContainer& GetDistances() const {
        return distances_between_stops_;
    }

private:
    std::deque<Stop> stops_ = {};
    std::unordered_map<const Stop*, BusesToStopNames> stop_buses_ = {};
    StopDistancesContainer distances_between_stops_ = {};
};

// ----------------------------------------------------------------------------

class BusCatalogue {
public:
    BusCatalogue() = default;

    const Bus* Push(std::string&& name, std::vector<std::string_view>&& string_rout, RoutType type,
        const VirtualStopCatalogue& stops_catalogue, const StopDistancesContainer& stops_distances);

private:
    double CalcRoutGeoLenght(const std::deque<const Stop*>& rout, RoutType rout_type);
    double CalcRoutTrueLenght(const std::deque<const Stop*>& rout, const StopDistancesContainer& stops_distances, RoutType rout_type);

private:
    std::deque<Bus> buses_ = {};
};

// ----------------------------------------------------------------------------

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddBus(std::string&& name, std::vector<std::string_view>&& rout, RoutType type) {
        const Bus* bus = buses_.Push(std::move(name), std::move(rout), type, virtual_stops_, stops_.GetDistances());
        virtual_buses_.Push(bus->name, bus);

        for (const Stop* stop : bus->rout) {
            stops_.PushBusToStop(stop, bus->name);
        }
    }

    void AddStop(std::string&& name, std::string&& string_coord) {
        const Stop* stop = stops_.Push(std::move(name), std::move(string_coord));
        virtual_stops_.Push(stop->name, stop);
    }

    auto GetBus(const std::string_view& name) const {
        return virtual_buses_.At(name);
    }

    std::pair<std::set<std::string_view>, bool> GetBusesForStop(const std::string_view& name) const {
        static const std::set<std::string_view> empty_set = {};
        auto [it, stop_has_been_found] = virtual_stops_.At(name);
        return (stop_has_been_found)
            ? std::make_pair(stops_.GetBuses((*it).second), stop_has_been_found)
            : std::make_pair(empty_set, stop_has_been_found);
    }

    void AddDistanceBetweenStops(const std::string_view& stop_from_name, const std::string_view& stop_to_name, double distance) {
        auto [stop_from_it, fist_stop_found] = virtual_stops_.At(stop_from_name);
        auto [stop_to_it, second_stop_found] = virtual_stops_.At(stop_to_name);
        stops_.AddDistance((*stop_from_it).second, (*stop_to_it).second, distance);
    }

private:
    BusCatalogue buses_;
    VirtualBusCatalogue virtual_buses_;

    StopCatalogue stops_;
    VirtualStopCatalogue virtual_stops_;
};

// ----------------------------------------------------------------------------
