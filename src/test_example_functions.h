#pragma once

#include <string_view>

#include "request_handler.h"

void TestTransportCatalogue();

void TestTransportCatalogueMakeBase(std::string_view file_name);

void TestTransportCatalogueProcessRequests(std::string_view file_name);
