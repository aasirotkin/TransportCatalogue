#include "serialization.h"

namespace transport_serialization {

void Serialization(std::ofstream& out, const transport_catalogue::TransportCatalogue& catalogue) {
    (void)out;
    (void)catalogue;
}

void Deserialization(transport_catalogue::TransportCatalogue& catalogue, std::ifstream& in) {
    (void)catalogue;
    (void)in;
}

} // namespace transport_serialization
