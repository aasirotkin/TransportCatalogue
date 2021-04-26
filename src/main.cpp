#include "request_handler.h"

#ifdef _SIROTKIN_HOME_TESTS_

#include "test_example_functions.h"

#if !defined(_WINDOWS_OS_) && !defined(_LINUX_OS_)
    #error "_WINDOWS_OS_ or _LINUX_OS_ must be defined in CMakeLists file"
#endif

#endif // _SIROTKIN_HOME_TESTS_

#include <iostream>
#include <string_view>

using namespace std::literals;

#ifdef _SIROTKIN_HOME_TESTS_

void SetOldTestFilePath() {
#ifdef _WINDOWS_OS_
    FilePathHelper::SetFilePathInput("C:\\Users\\aasir\\source\\repos\\aasirotkin\\TransportCatalogue\\tests\\input"s);
    FilePathHelper::SetFilePathOutput("C:\\Users\\aasir\\source\\repos\\aasirotkin\\TransportCatalogue\\tests\\output"s);
#elif defined _LINUX_OS_
    FilePathHelper::SetFilePathInput("/home/aasirotkin/Projects/TransportCatalogue/tests/input"s);
    FilePathHelper::SetFilePathOutput("/home/aasirotkin/Projects/TransportCatalogue/tests/output"s);
#endif
}

void SetMakeBaseFilePath() {
#ifdef _WINDOWS_OS_
    FilePathHelper::SetFilePathInput("C:\\Users\\aasir\\source\\repos\\aasirotkin\\TransportCatalogue\\tests\\make_base"s);
#elif defined _LINUX_OS_
    FilePathHelper::SetFilePathInput("/home/aasirotkin/Projects/TransportCatalogue/tests/make_base"s);
#endif
}

void SetProcessRequestsFilePath() {
#ifdef _WINDOWS_OS_
    FilePathHelper::SetFilePathInput("C:\\Users\\aasir\\source\\repos\\aasirotkin\\TransportCatalogue\\tests\\process_requests"s);
    FilePathHelper::SetFilePathOutput("C:\\Users\\aasir\\source\\repos\\aasirotkin\\TransportCatalogue\\tests\\process_requests_output"s);
#elif defined _LINUX_OS_
    FilePathHelper::SetFilePathInput("/home/aasirotkin/Projects/TransportCatalogue/tests/process_requests"s);
    FilePathHelper::SetFilePathOutput("/home/aasirotkin/Projects/TransportCatalogue/tests/process_requests_output"s);
#endif
}

int mainTests(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage of home tests: [make_base/process_requests/old_tests] [file_name (optional)]"sv << std::endl;
        return 1;
    }

    request_handler::ProgrammType type = request_handler::ParseProgrammType(argc, argv);
    if (type == request_handler::ProgrammType::UNKNOWN) {
        std::cerr << "Unknown file argument. Only [make_base/process_requests/old_tests] arguments are allowed"sv << std::endl;
        return 2;
    }

    if (type == request_handler::ProgrammType::OLD_TESTS) {
        SetOldTestFilePath();
        TestTransportCatalogue();
    }
    else {
        if (argc != 3) {
            std::cerr << "For arguments 'make_base' and 'process_requests' file_name is required"sv << std::endl;
            return 3;
        }

        std::string_view file_name(argv[2]);

        if (type == request_handler::ProgrammType::MAKE_BASE) {
            SetMakeBaseFilePath();
            TestTransportCatalogueMakeBase(file_name);
        }
        else {
            SetProcessRequestsFilePath();
            TestTransportCatalogueProcessRequests(file_name);
        }
    }

    return 0;
}

#endif

int mainPlatform(int argc, const char** argv) {
    request_handler::ProgrammType type = request_handler::ParseProgrammType(argc, argv);
    if (type == request_handler::ProgrammType::MAKE_BASE) {
        request_handler::RequestHandlerMakeBaseProcess(std::cin);
    }
    else if (type == request_handler::ProgrammType::PROCESS_REQUESTS) {
        request_handler::RequestHandlerProcessRequestProcess(std::cin, std::cout);
    }
    else {
        return 1;
    }

    return 0;
}

int main(int argc, const char** argv) {
#ifdef _SIROTKIN_HOME_TESTS_
    return mainTests(argc, argv);
#else
    return mainPlatform(argc, argv);
#endif
    return 0;
}

