#include "request_handler.h"

#include <iostream>
#include <string>
#include <string_view>

using namespace std::literals;

void MakeBase() {

}

void ProcessRequests() {

}

int main(int argc, const char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: "sv << argv[0] << " <argument>"sv << std::endl;
        return 1;
    }

    std::string argument = argv[1];
    if (argument == "make_base"sv) {
        MakeBase();
    }
    else if (argument == "process_requests"sv) {
        ProcessRequests();
    }
    else {
        std::cerr << "Usage: Argument can't be "sv << argument << std::endl;
        std::cerr << "Usage: Argument can only be 'make_base' or 'process_requests'"sv << std::endl;
        return 2;
    }

    return 0;
}
