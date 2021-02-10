#pragma once

#include <algorithm>
#include <deque>
#include <iostream>
#include <map>
#include <string_view>

#include "domain.h"
#include "geo.h"
#include "svg.h"

class MapRenderer : public svg::Document {
    using StopZoomedCoord = svg::Point;
public:
    MapRenderer();

    void CalculateZoomCoef(
        const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::stop_catalogue::Stop>& stops,
        const transport_catalogue::stop_catalogue::Catalogue& catalogue, double width, double height, double padding);

    void SetColorPalette(std::vector<svg::Color>&& color_palette);

    void InitStopsCoordForBuses(const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::bus_catalogue::Bus>& buses);

    void DrawLines(double line_width);

    void DrawText(int bus_label_font_size, svg::Point&& offset, svg::Color&& underlayer_color, double underlayer_width);

private:
    double zoom_coef_ = 0.0;
    double min_longitude_ = 0.0;
    double max_latitude_ = 0.0;
    double padding_ = 0.0;
    std::vector<svg::Color> color_palette_;
    std::map<std::string_view, bool> bus_is_back_and_forth_type_;
    std::map<std::string_view, std::deque<StopZoomedCoord>> bus_stop_coords_;
};
