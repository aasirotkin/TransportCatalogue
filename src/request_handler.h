#pragma once

#include "transport_catalogue.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

// ---------- Input queries ---------------------------------------------------

struct StopQuery {
    std::string_view name = {};
    std::string_view coord = {};
    std::vector<std::string_view> dist = {};

    StopQuery(const std::string_view& query);
};

std::string ReadLine(std::istream& input);

int ReadLineWithNumber(std::istream& input);

void InputQueries(transport_catalogue::TransportCatalogue& transport_catalogue, const std::vector<std::string>& queries);

// ---------- Output queries --------------------------------------------------

void OutputQuery(const transport_catalogue::TransportCatalogue& transport_catalogue, const std::string& query, std::ostream& out = std::cout);
