#include "request_handler.h"

#ifdef _SIROTKIN_HOME_TESTS_

#include "test_example_functions.h"

#endif // _SIROTKIN_HOME_TESTS_

#include <iostream>
#include <string_view>

using namespace std::literals;

int main(int argc, const char** argv) {
    request_handler::ProgrammType type = request_handler::ParseProgrammType(argc, argv);

#ifdef _SIROTKIN_HOME_TESTS_

    if (argc != 3) {
        return 2;
    }
    std::string_view file_name(argv[2]);

    if (type == request_handler::ProgrammType::MAKE_BASE) {
        TestTransportCatalogueMakeBase(file_name);
    }
    else if (type == request_handler::ProgrammType::PROCESS_REQUESTS) {
        TestTransportCatalogueProcessRequests(file_name);
    }
    else {
        return 1;
    }

#else

    if (type == request_handler::ProgrammType::MAKE_BASE) {
        request_handler::RequestHandlerMakeBaseProcess(std::cin);
    }
    else if (type == request_handler::ProgrammType::PROCESS_REQUESTS) {
        request_handler::RequestHandlerProcessRequestProcess(std::cin, std::cout);
    }
    else {
        return 1;
    }

#endif

    return 0;
}
