#pragma once

#include <algorithm>
#include <charconv>
#include <cmath>
#include <ostream>
#include <string>
#include <string_view>

struct Coordinates {
    double lat = 0;
    double lng = 0;

    static Coordinates ParseFromStringView(const std::string_view& string_coord) {
        size_t start_pos = string_coord.find_first_not_of(' ');
        size_t comma_pos = string_coord.find_first_of(',');
        size_t start_pos_next = string_coord.find_first_not_of(' ', comma_pos + 1);
        size_t end_pos = std::min(string_coord.find_first_of('\n'), string_coord.size());
        Coordinates coord;
        coord.lat = std::stod(std::string(string_coord).substr(start_pos, comma_pos));
        coord.lng = std::stod(std::string(string_coord).substr(start_pos_next, end_pos));
        //std::from_chars(string_coord.data() + start_pos, string_coord.data() + comma_pos, coord.lat);
        //std::from_chars(string_coord.data() + start_pos_next, string_coord.data() + end_pos, coord.lng);
        return coord;
    }
};

inline bool InTheVicinity(const double d1, const double d2, const double delta = 1e-6) {
    return abs(d1 - d2) < delta;
}

inline bool operator== (const Coordinates& lhs, const Coordinates& rhs) {
    return InTheVicinity(lhs.lat, rhs.lat) && InTheVicinity(lhs.lng, rhs.lng);
}

inline bool operator!= (const Coordinates& lhs, const Coordinates& rhs) {
    return !(lhs == rhs);
}

inline std::ostream& operator<< (std::ostream& out, const Coordinates& coord) {
    using namespace std::string_literals;
    out << "<|"s << coord.lat << ", "s << coord.lng << "|>"s;
    return out;
}

inline double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    static const double dr = 3.1415926535 / 180.0;
    static const double rz = 6371000.0;
    return acos(
        sin(from.lat * dr) * sin(to.lat * dr) +
        cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr)) * rz;
}
