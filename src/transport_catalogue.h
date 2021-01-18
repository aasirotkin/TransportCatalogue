#pragma once

#include "geo.h"

#include <algorithm>
#include <deque>
#include <iomanip>
#include <numeric>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
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

struct Bus {
    std::string name = {};
    std::deque<const Stop*> rout = {};
    RoutType rout_type = RoutType::Direct;
    int stops_on_rout = 0;
    int unique_stops = 0;
    double rout_lenght = 0.0;

    Bus() = default;

    Bus(std::string&& name, std::deque<const Stop*>&& rout, RoutType rout_type)
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
};

inline std::ostream& operator<< (std::ostream& out, const Bus& bus) {
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

class StopCatalogue : public std::deque<Stop> {
public:
    StopCatalogue() = default;

    const Stop* push(std::string&& name, std::string&& string_coord) {
        push_back({ std::move(name), Coordinates::ParseFromStringView(string_coord) });
        return &back();
    }
};

class BusCatalogue : public std::deque<Bus> {
public:
    BusCatalogue() = default;

    const Bus* push(std::string&& name, std::vector<std::string_view>&& string_rout, RoutType type, const VirtualStopCatalogue& stops_catalogue) {
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
};

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
