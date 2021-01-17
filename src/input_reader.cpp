#include "input_reader.h"

std::string ReadLine(std::istream& input) {
    std::string s;
    std::getline(input, s);
    return s;
}

int ReadLineWithNumber(std::istream& input) {
    int number = 0;
    input >> number;
    ReadLine(input);
    return number;
}

void InputQueries(TransportCatalogue& transport_catalogue, const std::vector<std::string>& queries) {
    (void)transport_catalogue;
    (void)queries;
}
