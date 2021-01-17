#include "stat_reader.h"

void OutputQuery(const TransportCatalogue& transport_catalogue, const std::string& query, std::ostream& out) {
    std::string bus_name = query.substr(query.find_first_of(' ') + 1);

    auto [it, bus_has_been_found] = transport_catalogue.GetBus(bus_name);

    if (bus_has_been_found) {
        out << *it;
    }
    else {
        out << std::string("Bus ") << bus_name << std::string(": not found");
    }
    out << "\n";
}
