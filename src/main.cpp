#include "request_handler.h"

#ifdef _SIROTKIN_HOME_TESTS_

#include "test_example_functions.h"

#endif // _SIROTKIN_HOME_TESTS_

#include <iostream>
#include <string_view>

using namespace std::literals;

int main(int argc, const char** argv) {
    request_handler::ProgrammType type = request_handler::ParseProgrammType(argc, argv);

    if (type == request_handler::ProgrammType::MAKE_BASE) {
#ifdef _SIROTKIN_HOME_TESTS_
        TestTransportCatalogueMakeBase();
#else
        request_handler::RequestHandlerMakeBaseProcess(std::cin);
#endif
    }
    else if (type == request_handler::ProgrammType::PROCESS_REQUEST) {
#ifdef _SIROTKIN_HOME_TESTS_
        TestTransportCatalogueProcessRequest();
#else
        request_handler::RequestHandlerProcessRequestProcess(std::cin, std::cout);
#endif
    }
    else {
        return 1;
    }

    return 0;
}
