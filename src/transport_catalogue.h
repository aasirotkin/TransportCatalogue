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

    bool operator==(const Stop& other) const {
        return name == other.name;
    }
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

    bool operator==(const Bus& other) const {
        return name == other.name;
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

template <typename StructWithName>
class CatalogueHash {
public:
    size_t operator() (const StructWithName& struct_with_name) const {
        return hasher_(struct_with_name.name);
    }
private:
    std::hash<std::string> hasher_;
};

using StopHasher = CatalogueHash<Stop>;
using BusHasher = CatalogueHash<Bus>;

template <typename StructWithName, typename StructHasher>
class Catalogue {
public:
    Catalogue() = default;

    std::pair<typename std::unordered_set<StructWithName, StructHasher>::const_iterator, bool> at(const std::string_view& name) const {
        auto it = std::find_if(data_.begin(), data_.end(), [&name](const StructWithName& value) { return value.name == name; });
        return std::make_pair(it, (it == data_.end() ? false : true));
    }

    std::pair<typename std::unordered_set<StructWithName, StructHasher>::const_iterator, bool> push(StructWithName&& struct_with_name) {
        return data_.insert(std::move(struct_with_name));
    }
private:
    std::unordered_set<StructWithName, StructHasher> data_;
};

class StopCatalogue : public Catalogue<Stop, StopHasher> {
public:
    StopCatalogue() = default;

    auto push(std::string&& name, std::string&& string_coord) {
        return Catalogue<Stop, StopHasher>::push({ name, Coordinates::ParseFromStringView(string_coord) });
    }
};

class BusCatalogue : public Catalogue<Bus, BusHasher> {
public:
    BusCatalogue() = default;

    auto push(std::string&& name, std::vector<std::string_view>&& string_rout, RoutType type, const StopCatalogue& stops_catalogue) {
        std::deque<const Stop*> stops;
        for (std::string_view stop_name : string_rout) {
            auto [it, res] = stops_catalogue.at(stop_name);
            if (res) {
                stops.push_back(&(*it));
            }
        }
        return Catalogue<Bus, BusHasher>::push(Bus(std::move(name), std::move(stops), type));
    }
};

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddBus(std::string&& name, std::vector<std::string_view>&& string_rout_container, RoutType type) {
        buses_.push(std::move(name), std::move(string_rout_container), type, stops_);
    }

    void AddStop(std::string&& name, std::string&& string_coord) {
        stops_.push(std::move(name), std::move(string_coord));
    }

    auto GetBus(const std::string& name) const {
        return buses_.at(name);
    }

private:
    StopCatalogue stops_;
    BusCatalogue buses_;
};
