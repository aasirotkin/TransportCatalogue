#include "request_handler.h"

#ifdef _SIROTKIN_HOME_TESTS_

#include "test_example_functions.h"

#endif // _SIROTKIN_HOME_TESTS_


#include <iostream>

int main() {

#ifdef _SIROTKIN_HOME_TESTS_

    TestTransportCatalogue();

#else

    request_handler::RequestHandlerProcess(std::cin, std::cout);

#endif

    return 0;
}
