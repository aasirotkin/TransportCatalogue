#include "map_renderer.h"

#include <algorithm>

MapRenderer::MapRenderer() {
}

void MapRenderer::CalculateZoomCoef(
    const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::stop_catalogue::Stop>& stops,
    const transport_catalogue::stop_catalogue::Catalogue& catalogue, double width, double height, double padding) {
    double min_lat = 0.0;
    double max_lat = 0.0;
    double min_lon = 0.0;
    double max_lon = 0.0;
    bool coordinates_is_inited = false;
    for (const auto& [name, stop] : stops) {
        if (!catalogue.IsEmpty(stop)) {
            if (!coordinates_is_inited) {
                min_lat = stop->coord.lat;
                max_lat = stop->coord.lat;
                min_lon = stop->coord.lng;
                max_lon = stop->coord.lng;
                coordinates_is_inited = true;
            }
            else {
                if (min_lat > stop->coord.lat) {
                    min_lat = stop->coord.lat;
                }
                if (max_lat < stop->coord.lat) {
                    max_lat = stop->coord.lat;
                }
                if (min_lon > stop->coord.lng) {
                    min_lon = stop->coord.lng;
                }
                if (max_lon < stop->coord.lng) {
                    max_lon = stop->coord.lng;
                }
            }
        }
    }

    double delta_lat = max_lat - min_lat;
    double delta_lon = max_lon - min_lon;

    double height_coef = (std::abs(delta_lat) > 1e-6) ? ((height - 2.0 * padding) / (max_lat - min_lat)) : 0.0;
    double width_coef = (std::abs(delta_lon) > 1e-6) ?  ((width - 2.0 * padding) / (max_lon - min_lon)) : 0.0;

    zoom_coef_ = std::min(height_coef, width_coef);
    min_longitude_ = min_lon;
    max_latitude_ = max_lat;
    padding_ = padding;
}

void MapRenderer::SetColorPalette(std::vector<svg::Color>&& color_palette) {
    color_palette_ = std::move(color_palette);
}

void MapRenderer::InitStopsCoordForBuses(const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::bus_catalogue::Bus>& buses) {
    for (const auto& [name, bus] : buses) {
        if (bus->stops_on_route > 0) {
            auto name_deque_it = bus_stop_coords_.emplace(name, std::deque<StopZoomedCoord>{});
            auto& coords = (name_deque_it.first)->second;

            bus_is_back_and_forth_type_.emplace(name, bus->route_type == transport_catalogue::RouteType::BackAndForth);

            for (const transport_catalogue::stop_catalogue::Stop* stop : bus->route) {
                double x = (stop->coord.lng - min_longitude_) * zoom_coef_ + padding_;
                double y = (max_latitude_ - stop->coord.lat) * zoom_coef_ + padding_;

                coords.push_back({ x,y });
            }
        }
    }
}

void MapRenderer::DrawLines(double line_width) {
    size_t color_index = 0;
    size_t sorted_buses_size = bus_stop_coords_.size();
    for (const auto& [name, coords] : bus_stop_coords_) {
        svg::Polyline polyline;
        polyline.SetFillColor(svg::Color());
        polyline.SetStrokeWidth(line_width);
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        polyline.SetStrokeColor(color_palette_.at(color_index));
        ++color_index;
        if (color_index == sorted_buses_size) {
            color_index = 0;
        }

        for (const StopZoomedCoord& coord : coords) {
            polyline.AddPoint(coord);
        }

        if (bus_is_back_and_forth_type_.at(name)) {
            for (auto it = coords.rbegin() + 1; it != coords.rend(); ++it) {
                polyline.AddPoint(*it);
            }
        }

        Add(std::move(polyline));
    }
}

void MapRenderer::DrawText(int bus_label_font_size, svg::Point&& offset, svg::Color&& underlayer_color, double underlayer_width) {
    using namespace std::literals;

    svg::Text text;
    text.SetOffset(offset);
    text.SetFontSize(bus_label_font_size);
    text.SetFontFamily("Verdana"s);
    text.SetFontWeight("bold"s);

    size_t color_index = 0;
    size_t sorted_buses_size = bus_stop_coords_.size();
    for (const auto& [name, coords] : bus_stop_coords_) {
        text.SetData(std::string(name));

        svg::Text underlayer_text = text;

        underlayer_text.SetFillColor(underlayer_color);
        underlayer_text.SetStrokeColor(underlayer_color);
        underlayer_text.SetStrokeWidth(underlayer_width);
        underlayer_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text data_text = text;
        text.SetFillColor(color_palette_.at(color_index));
        ++color_index;
        if (color_index == sorted_buses_size) {
            color_index = 0;
        }

        underlayer_text.SetPosition(coords.front());
        data_text.SetPosition(coords.front());

        Add(underlayer_text);
        Add(data_text);

        if (bus_is_back_and_forth_type_.at(name) && coords.front() != coords.back()) {
            underlayer_text.SetPosition(coords.back());
            data_text.SetPosition(coords.back());

            Add(std::move(underlayer_text));
            Add(std::move(data_text));
        }
    }
}
