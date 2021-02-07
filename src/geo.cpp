#include "geo.h"

#include <algorithm>
#include <cmath>
#include <string>

inline bool InTheVicinity(const double d1, const double d2, const double delta = 1e-6) {
    return abs(d1 - d2) < delta;
}

Coordinates Coordinates::ParseFromStringView(const std::string_view& string_coord) {
    size_t start_pos = string_coord.find_first_not_of(' ');
    size_t comma_pos = string_coord.find_first_of(',');
    size_t start_pos_next = string_coord.find_first_not_of(' ', comma_pos + 1);
    Coordinates coord;
    coord.lat = std::stod(std::string(string_coord.substr(start_pos, comma_pos)));
    coord.lng = std::stod(std::string(string_coord.substr(start_pos_next)));
    return coord;
}

bool operator==(const Coordinates& lhs, const Coordinates& rhs) {
    return InTheVicinity(lhs.lat, rhs.lat) && InTheVicinity(lhs.lng, rhs.lng);
}

bool operator!=(const Coordinates& lhs, const Coordinates& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const Coordinates& coord) {
    using namespace std::string_literals;
    out << "<|"s << coord.lat << ", "s << coord.lng << "|>"s;
    return out;
}

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    static const double dr = 3.1415926535 / 180.0;
    static const double rz = 6371000.0;
    return acos(
        sin(from.lat * dr) * sin(to.lat * dr) +
        cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr)) * rz;
}
