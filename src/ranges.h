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

        Range(const It& begin, const It& end)
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
        const It& begin_;
        const It& end_;
    };

    template <typename C>
    auto AsRange(const C& container) {
        return Range{ container.begin(), container.end() };
    }

// ----------------------------------------------------------------------------

    template <typename ConstIterator>
    struct GraphBusRange {
        const transport_catalogue::bus_catalogue::Bus* bus_ptr;
        const ConstIterator& route_begin;
        const ConstIterator& route_end;

        GraphBusRange(
            const transport_catalogue::bus_catalogue::Bus* bus,
            const ConstIterator& begin,
            const ConstIterator& end)
            : bus_ptr(bus)
            , route_begin(begin)
            , route_end(end) {
        }
    };

    template <typename RouteContainer>
    auto BusRangeDirect(const transport_catalogue::bus_catalogue::Bus* bus, const RouteContainer& route) {
        return GraphBusRange {
            bus, route.begin(), route.end()
        };
    }

    template <typename RouteContainer>
    auto BusRangeReversed(const transport_catalogue::bus_catalogue::Bus* bus, const RouteContainer& route) {
        return GraphBusRange {
            bus, route.rbegin(), route.rend()
        };
    }

}  // namespace ranges
