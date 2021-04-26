#pragma once

#include <filesystem>
#include <string_view>

class FilePathHelper {
public:
    static std::filesystem::path PathInput() {
        return path_in_;
    }

    static std::filesystem::path PathOutput() {
        return path_out_;
    }

    static void SetFilePathInput(std::filesystem::path path) {
        path_in_ = std::move(path);
    }

    static void SetFilePathOutput(std::filesystem::path path) {
        path_out_ = std::move(path);
    }

private:
    static std::filesystem::path path_in_;
    static std::filesystem::path path_out_;
};

std::filesystem::path operator""_p(const char* data, std::size_t sz);

void TestTransportCatalogue();

void TestTransportCatalogueMakeBase(std::string_view file_name);

void TestTransportCatalogueProcessRequests(std::string_view file_name);
