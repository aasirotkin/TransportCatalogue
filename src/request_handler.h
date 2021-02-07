#pragma once

#include <istream>
#include <ostream>
#include <string_view>
#include <vector>

namespace request_handler {

namespace detail {

struct StopRequest {
    std::string_view name = {};
    std::string_view coord = {};
    std::vector<std::string_view> dist = {};

    StopRequest(const std::string_view & request);
};

} // namespace parse_detail

void RequestHandler(std::istream& input, std::ostream& output);

} // namespace request_handler
