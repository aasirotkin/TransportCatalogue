#include <transport.pb.h>

#include <fstream>

#include "request_handler.h"

namespace transport_serialization {

void Serialization(std::ofstream& out, const request_handler::RequestHandler& request_handler);

void Deserialization(request_handler::RequestHandler& request_handler, std::ifstream& in);

} // namespace transport_serialization
