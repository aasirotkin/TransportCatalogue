#include <transport.pb.h>

#include <fstream>

#include "transport_catalogue.h"

namespace transport_serialization {

void Serialization(std::ofstream& out, const transport_catalogue::TransportCatalogue& catalogue);

void Deserialization(transport_catalogue::TransportCatalogue& catalogue, std::ifstream& in);

} // namespace transport_serialization
