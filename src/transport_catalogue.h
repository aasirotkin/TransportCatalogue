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
    size_t stops_on_rout = 0;
    size_t unique_stops = 0;

    Bus() = default;

    Bus(std::string&& name, std::deque<const Stop*>&& rout, RoutType rout_type);

private:
    double CalcRoutLenght(RoutType rout_type);
};

std::ostream& operator<< (std::ostream& out, const Bus& bus);

// ----------------------------------------------------------------------------

template <typename VirtualStruct>
class VirtualCatalogue {
public:
    VirtualCatalogue() = default;

    auto at(const std::string_view& name) const {
        auto it = data_.find(name);
        return std::make_pair(it, (it == data_.end() ? false : true));
    }

    auto push(const std::string_view& name, const VirtualStruct* data) {
        return data_.emplace(name, data);
    }
private:
    std::unordered_map<std::string_view, const VirtualStruct*> data_;
};

using VirtualStopCatalogue = VirtualCatalogue<Stop>;
using VirtualBusCatalogue = VirtualCatalogue<Bus>;

// ----------------------------------------------------------------------------

using StopsReferencePair = std::pair<const Stop*, const Stop*>;

class StopsReferencePairHasher {
public:
    size_t operator() (const StopsReferencePair& stop_pair) const {
        return hasher_(stop_pair.first) * 47 + hasher_(stop_pair.second);
    }
private:
    std::hash<const Stop*> hasher_;
};

// ----------------------------------------------------------------------------

using BusesToStopNames = std::set<std::string_view>;

std::ostream& operator<< (std::ostream& out, const BusesToStopNames& buses);

class StopCatalogue : public std::deque<Stop> {
public:
    StopCatalogue() = default;

    const Stop* push(std::string&& name, std::string&& string_coord);

    void push_bus_to_stop(const Stop* stop, const std::string_view& bus_name);

    const std::set<std::string_view>& get_buses(const Stop* stop) const {
        return stop_buses_.at(stop);
    }

    void add_distance(const Stop* stop_1, const Stop* stop_2, double distance) {
        StopsReferencePair stop_pair_direct = { stop_1, stop_2 };
        StopsReferencePair stop_pair_reverse = { stop_2, stop_1 };

        distances_between_stops_[stop_pair_direct] = distance;

        if (distances_between_stops_.count(stop_pair_reverse) == 0) {
            distances_between_stops_[stop_pair_reverse] = distance;
        }
    }

private:
    std::unordered_map<const Stop*, BusesToStopNames> stop_buses_ = {};
    std::unordered_map<StopsReferencePair, double, StopsReferencePairHasher> distances_between_stops_ = {};
};

// ----------------------------------------------------------------------------

class BusCatalogue : public std::deque<Bus> {
public:
    BusCatalogue() = default;

    const Bus* push(std::string&& name, std::vector<std::string_view>&& string_rout, RoutType type, const VirtualStopCatalogue& stops_catalogue);
};

// ----------------------------------------------------------------------------

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddBus(std::string&& name, std::vector<std::string_view>&& rout, RoutType type) {
        const Bus* bus = buses_.push(std::move(name), std::move(rout), type, virtual_stops_);
        virtual_buses_.push(bus->name, bus);

        for (const Stop* stop : bus->rout) {
            stops_.push_bus_to_stop(stop, bus->name);
        }
    }

    void AddStop(std::string&& name, std::string&& string_coord) {
        const Stop* stop = stops_.push(std::move(name), std::move(string_coord));
        virtual_stops_.push(stop->name, stop);
    }

    auto GetBus(const std::string_view& name) const {
        return virtual_buses_.at(name);
    }

    auto GetStop(const std::string_view& name) const {
        static const std::set<std::string_view> empty_set = {};
        auto [it, stop_has_been_found] = virtual_stops_.at(name);
        return (stop_has_been_found)
            ? std::make_pair(stops_.get_buses((*it).second), stop_has_been_found)
            : std::make_pair(empty_set, stop_has_been_found);
    }

    void AddDistanceBetweenStops(const std::string_view& stop_from_name, const std::string_view& stop_to_name, double distance) {
        auto [stop_from_it, fist_stop_found] = virtual_stops_.at(stop_from_name);
        auto [stop_to_it, second_stop_found] = virtual_stops_.at(stop_to_name);
        stops_.add_distance((*stop_from_it).second, (*stop_to_it).second, distance);
    }

private:
    BusCatalogue buses_;
    VirtualBusCatalogue virtual_buses_;

    StopCatalogue stops_;
    VirtualStopCatalogue virtual_stops_;
};

// ----------------------------------------------------------------------------
