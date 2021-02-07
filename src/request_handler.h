#pragma once

#include <istream>
#include <ostream>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace request_handler {

namespace detail {

struct StopRequest {
    std::string_view name = {};
    std::string_view coord = {};
    std::vector<std::string_view> dist = {};

    StopRequest(const std::string_view & request);
};

struct JsonStopRequest {
    std::string_view name = {};
    double latitude = 0.0;
    double longitude = 0.0;
    std::unordered_map<std::string_view, double> road_distances = {};
};

struct JsonBusRequest {
    std::string_view name = {};
    std::unordered_set<std::string_view> stops = {};
    bool is_roundtrip = false;
};

} // namespace parse_detail

void RequestHandler(std::istream& input, std::ostream& output);

} // namespace request_handler
