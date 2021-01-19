#pragma once

#include "geo.h"

#include <algorithm>
#include <deque>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <vector>

struct Stop {
    std::string name;
    Coordinates coord;
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
    int stops_on_rout = 0;
    int unique_stops = 0;

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

class StopCatalogue : public std::deque<Stop> {
public:
    StopCatalogue() = default;

    const Stop* push(std::string&& name, std::string&& string_coord);
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
    }

    void AddStop(std::string&& name, std::string&& string_coord) {
        const Stop* stop = stops_.push(std::move(name), std::move(string_coord));
        virtual_stops_.push(stop->name, stop);
    }

    auto GetBus(const std::string_view& name) const {
        return virtual_buses_.at(name);
    }

private:
    BusCatalogue buses_;
    VirtualBusCatalogue virtual_buses_;

    StopCatalogue stops_;
    VirtualStopCatalogue virtual_stops_;
};

// ----------------------------------------------------------------------------
