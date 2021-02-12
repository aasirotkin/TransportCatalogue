#include "map_renderer.h"

#include <algorithm>
#include <utility>

MapRenderer::MapRenderer(
    MapRendererSettings&& render_settings,
    const transport_catalogue::stop_catalogue::Catalogue& stop_catalogue,
    const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::stop_catalogue::Stop>& stops,
    const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::bus_catalogue::Bus>& buses)
    : render_settings_(std::move(render_settings)) {
    InitNotEmptyStops(stop_catalogue, stops);
    InitNotEmptyBuses(buses);
    CalculateZoomCoef();
    CalculateStopZoomedCoords();
    DrawLines();
    DrawBusText();
    DrawStopCircles();
    DrawStopText();
}

void MapRenderer::InitNotEmptyStops(
    const transport_catalogue::stop_catalogue::Catalogue& catalogue,
    const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::stop_catalogue::Stop>& stops) {
    for (const auto& [name, stop] : stops) {
        if (!catalogue.IsEmpty(stop)) {
            stop_point_.emplace(stop, svg::Point{});
            stops_.emplace(name, stop);
        }
    }
}

void MapRenderer::InitNotEmptyBuses(
    const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::bus_catalogue::Bus>& buses) {
    for (const auto& [name, bus] : buses) {
        if (bus->stops_on_route > 0) {
            buses_.emplace(name, bus);
        }
    }
}

void MapRenderer::CalculateZoomCoef() {
    double min_lat = 0.0;
    double max_lat = 0.0;
    double min_lon = 0.0;
    double max_lon = 0.0;
    bool coordinates_is_inited = false;
    for (const auto& [stop, point] : stop_point_) {
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

    double delta_lat = max_lat - min_lat;
    double delta_lon = max_lon - min_lon;

    double height_coef = (std::abs(delta_lat) > 1e-6) ? ((render_settings_.height - 2.0 * render_settings_.padding) / delta_lat) : 0.0;
    double width_coef = (std::abs(delta_lon) > 1e-6) ?  ((render_settings_.width - 2.0 * render_settings_.padding) / delta_lon) : 0.0;

    zoom_coef_ = std::min(height_coef, width_coef);
    min_longitude_ = min_lon;
    max_latitude_ = max_lat;
}

void MapRenderer::CalculateStopZoomedCoords() {
    for (auto& [stop, point] : stop_point_) {
        point.x = (stop->coord.lng - min_longitude_) * zoom_coef_ + render_settings_.padding;
        point.y = (max_latitude_ - stop->coord.lat) * zoom_coef_ + render_settings_.padding;
    }
}

void MapRenderer::DrawLines() {
    size_t color_index = 0;
    size_t color_palette_size = render_settings_.color_palette.size();
    for (const auto& [name, bus] : buses_) {
        svg::Polyline polyline;
        polyline.SetFillColor(svg::Color());
        polyline.SetStrokeWidth(render_settings_.line_width);
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        polyline.SetStrokeColor(render_settings_.color_palette.at(color_index));
        ++color_index;
        if (color_index == color_palette_size) {
            color_index = 0;
        }

        for (const auto& stop : bus->route) {
            polyline.AddPoint(stop_point_.at(stop));
        }

        if (bus->route_type == transport_catalogue::RouteType::BackAndForth) {
            for (auto it = bus->route.rbegin() + 1; it != bus->route.rend(); ++it) {
                polyline.AddPoint(stop_point_.at(*it));
            }
        }

        Add(std::move(polyline));
    }
}

void MapRenderer::DrawBusText() {
    using namespace std::literals;

    svg::Text text;
    text.SetOffset(render_settings_.bus_label_offset);
    text.SetFontSize(render_settings_.bus_label_font_size);
    text.SetFontFamily("Verdana"s);
    text.SetFontWeight("bold"s);

    size_t color_index = 0;
    size_t color_palette_size = render_settings_.color_palette.size();
    for (const auto& [name, bus] : buses_) {
        text.SetData(std::string(name));

        svg::Text underlayer_text = text;
        underlayer_text.SetFillColor(render_settings_.underlayer_color);
        underlayer_text.SetStrokeColor(render_settings_.underlayer_color);
        underlayer_text.SetStrokeWidth(render_settings_.underlayer_width);
        underlayer_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text data_text = text;
        data_text.SetFillColor(render_settings_.color_palette.at(color_index));
        ++color_index;
        if (color_index == color_palette_size) {
            color_index = 0;
        }

        underlayer_text.SetPosition(stop_point_.at(bus->route.front()));
        data_text.SetPosition(stop_point_.at(bus->route.front()));

        Add(underlayer_text);
        Add(data_text);

        if (bus->route_type == transport_catalogue::RouteType::BackAndForth && *bus->route.front() != *bus->route.back()) {
            underlayer_text.SetPosition(stop_point_.at(bus->route.back()));
            data_text.SetPosition(stop_point_.at(bus->route.back()));

            Add(std::move(underlayer_text));
            Add(std::move(data_text));
        }
    }
}

void MapRenderer::DrawStopCircles() {
    using namespace std::string_literals;

    for (const auto& [name, stop] : stops_) {
        svg::Circle circle;
        circle.SetCenter(stop_point_.at(stop));
        circle.SetRadius(render_settings_.stop_radius);
        circle.SetFillColor(svg::Color("white"s));

        Add(std::move(circle));
    }
}

void MapRenderer::DrawStopText() {
    using namespace std::literals;

    svg::Text text;
    text.SetOffset(render_settings_.stop_label_offset);
    text.SetFontSize(render_settings_.stop_label_font_size);
    text.SetFontFamily("Verdana"s);

    for (const auto& [name, stop] : stops_) {
        text.SetData(std::string(name));
        text.SetPosition(stop_point_.at(stop));

        svg::Text underlayer_text = text;
        underlayer_text.SetFillColor(render_settings_.underlayer_color);
        underlayer_text.SetStrokeColor(render_settings_.underlayer_color);
        underlayer_text.SetStrokeWidth(render_settings_.underlayer_width);
        underlayer_text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        underlayer_text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        svg::Text data_text = text;
        data_text.SetFillColor(svg::Color("black"s));

        Add(std::move(underlayer_text));
        Add(std::move(data_text));
    }
}
