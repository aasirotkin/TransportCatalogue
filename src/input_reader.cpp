#include "input_reader.h"

#include "geo.h"

#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

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

std::vector<std::string_view> SplitIntoWords(std::string_view text, char sep = ' ')
{
    std::vector<std::string_view> words;
    while (true) {
        size_t pos_begin = text.find_first_not_of(sep);
        size_t pos_end = std::min(text.find_first_of(sep, pos_begin + 1), text.size());
        if (pos_begin == std::string_view::npos) {
            break;
        }
        words.push_back(text.substr(pos_begin, pos_end - pos_begin));
        text.remove_prefix(pos_end);
    }
    return words;
}

std::pair<std::string_view, std::string_view> ParseStringOnNameData(const std::string_view& query) {
    size_t start_name_pos = query.find_first_of(' ') + 1;
    size_t end_name_pos = query.find_first_of(':', start_name_pos);
    size_t start_coord_pos = end_name_pos + 1;
    size_t end_coord_pos = query.size();
    return {
        query.substr(start_name_pos, end_name_pos - start_name_pos),
        query.substr(start_coord_pos, end_coord_pos) };
}

void InputStop(TransportCatalogue& transport_catalogue, const std::string_view& string_stop) {
    auto [name, data] = ParseStringOnNameData(string_stop);
    transport_catalogue.AddStop(std::string(name), std::string(data));
}

void InputBus(TransportCatalogue& transport_catalogue, const std::string_view& string_bus) {
    auto [name, data] = ParseStringOnNameData(string_bus);

    RoutType type = (data.find('>') != std::string_view::npos) ? RoutType::Round : RoutType::BackAndForth;
    char sep = (type == RoutType::Round) ? '>' : '-';

    std::vector<std::string_view> rout = SplitIntoWords(data, sep);

    for (std::string_view& rout_i : rout) {
        rout_i.remove_prefix(rout_i.find_first_not_of(' '));
        rout_i.remove_suffix(rout_i.size() - rout_i.find_last_not_of(' ') - 1);
    }

    transport_catalogue.AddBus(std::string(name), std::move(rout), type);
}

void InputQueries(TransportCatalogue& transport_catalogue, const std::vector<std::string>& queries) {
    std::vector<std::string_view> stop_query;
    std::vector<std::string_view> bus_query;
    for (const std::string& query : queries) {
        if (query.rfind(std::string("Stop"), 0) == 0) {
            stop_query.push_back(query);
        }
        else {
            bus_query.push_back(query);
        }
    }

    for (const std::string_view& query : stop_query) {
        InputStop(transport_catalogue, query);
    }

    for (const std::string_view& query : bus_query) {
        InputBus(transport_catalogue, query);
    }
}
