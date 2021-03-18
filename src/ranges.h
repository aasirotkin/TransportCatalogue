#pragma once

#include "domain.h"

#include <iterator>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace ranges {

    template <typename It>
    class Range {
    public:
        using ValueType = typename std::iterator_traits<It>::value_type;

        Range(It begin, It end)
            : begin_(begin)
            , end_(end) {
        }

        const It& begin() const {
            return begin_;
        }

        const It& end() const {
            return end_;
        }

    private:
        It begin_;
        It end_;
    };

    template <typename C>
    inline auto AsRange(const C& container) {
        return Range{ container.begin(), container.end() };
    }

// ----------------------------------------------------------------------------

    template <typename ConstIterator>
    struct GraphBusRange {
        const transport_catalogue::bus_catalogue::Bus* bus_ptr;
        ConstIterator route_begin;
        ConstIterator route_end;

        GraphBusRange(
            const transport_catalogue::bus_catalogue::Bus* bus,
            ConstIterator begin,
            ConstIterator end)
            : bus_ptr(bus)
            , route_begin(begin)
            , route_end(end) {
        }
    };

    inline auto BusRangeDirect(const transport_catalogue::bus_catalogue::Bus* bus) {
        return GraphBusRange {
            bus, bus->route.begin(), bus->route.end()
        };
    }

    inline auto BusRangeReversed(const transport_catalogue::bus_catalogue::Bus* bus) {
        return GraphBusRange {
            bus, bus->route.rbegin(), bus->route.rend()
        };
    }

}  // namespace ranges
