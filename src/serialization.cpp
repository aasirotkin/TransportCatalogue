#include "serialization.h"

#include <iostream>

#include "map_renderer.h"
#include "svg.h"
#include "variant"

namespace transport_serialization {

transport_proto::Coordinates CreateProtoCoord(const Coordinates& coord) {
    transport_proto::Coordinates proto_coord;
    proto_coord.set_lat(coord.lat);
    proto_coord.set_lng(coord.lng);
    return proto_coord;
}

transport_proto::Stop CreateProtoStop(const transport_catalogue::stop_catalogue::Stop* stop) {
    transport_proto::Stop proto_stop;

    proto_stop.set_id(stop->id);
    proto_stop.set_name(stop->name);
    *proto_stop.mutable_coord() = CreateProtoCoord(stop->coord);

    return proto_stop;
}

transport_proto::Bus CreateProtoBus(const transport_catalogue::bus_catalogue::Bus* bus) {
    transport_proto::Bus proto_bus;

    proto_bus.set_name(bus->name);
    for (const auto* stop : bus->route) {
        proto_bus.add_route(stop->id);
    }
    proto_bus.set_type(static_cast<uint32_t>(bus->route_type));
    proto_bus.set_route_geo_length(bus->route_geo_length);
    proto_bus.set_route_true_length(bus->route_true_length);
    proto_bus.set_stops_on_route(bus->stops_on_route);
    proto_bus.set_unique_stops(bus->unique_stops);

    return proto_bus;
}

transport_proto::RouteSettings CreateProtoRouteSetting(const transport_catalogue::RouteSettings& settings) {
    transport_proto::RouteSettings proto_settings;

    proto_settings.set_bus_velocity(settings.bus_velocity);
    proto_settings.set_bus_waiting_time(settings.bus_wait_time);

    return proto_settings;
}

transport_proto::Point CreateProtoPoint(const svg::Point& point) {
    transport_proto::Point proto_point;

    proto_point.set_x(point.x);
    proto_point.set_y(point.y);

    return proto_point;
}

transport_proto::Color CreateProtoColor(const svg::Color& color) {
    transport_proto::Color proto_color;

    proto_color.set_type(svg::ColorTypeToInt(svg::GetColorType(color)));
    proto_color.set_name(svg::GetColorStringName(color));

    return proto_color;
}

transport_proto::MapRenderSettings CreateProtoMapRenderSettings(const map_renderer::MapRendererSettings& setting) {
    transport_proto::MapRenderSettings proto_settings;

    proto_settings.set_width(setting.width);
    proto_settings.set_height(setting.height);
    proto_settings.set_padding(setting.padding);
    proto_settings.set_line_width(setting.line_width);
    proto_settings.set_stop_radius(setting.stop_radius);
    proto_settings.set_bus_label_font_size(setting.bus_label_font_size);
    *proto_settings.mutable_bus_label_offset() = CreateProtoPoint(setting.bus_label_offset);
    proto_settings.set_stop_label_font_size(setting.stop_label_font_size);
    *proto_settings.mutable_stop_label_offset() = CreateProtoPoint(setting.stop_label_offset);
    *proto_settings.mutable_underlayer_color() = CreateProtoColor(setting.underlayer_color);
    proto_settings.set_underlayer_width(setting.underlayer_width);
    for (const svg::Color& color : setting.color_palette) {
        *proto_settings.add_color_palette() = CreateProtoColor(color);
    }

    return proto_settings;
}

void Serialization(std::ofstream& out, const request_handler::RequestHandler& request_handler) {
    transport_proto::TransportCatalogue tc;

    for (const transport_catalogue::stop_catalogue::Stop* stop : request_handler.GetStops()) {
        *tc.add_stop() = CreateProtoStop(stop);
    }

    for (const transport_catalogue::bus_catalogue::Bus* bus : request_handler.GetBuses()) {
        *tc.add_bus() = CreateProtoBus(bus);
    }

    *tc.mutable_map_render_setting() = CreateProtoMapRenderSettings(request_handler.GetMapRenderSettings().value());

    // *tc.mutable_route_settings() = CreateProtoRouteSetting(request_handler.GetRouteSettings());

    tc.SerializeToOstream(&out);
}

Coordinates CreateCoord(const transport_proto::Coordinates& proto_coord) {
    Coordinates coord;

    coord.lat = proto_coord.lat();
    coord.lng = proto_coord.lng();

    return coord;
}

transport_catalogue::stop_catalogue::Stop CreateStop(const transport_proto::Stop& proto_stop) {
    transport_catalogue::stop_catalogue::Stop stop;

    stop.id = proto_stop.id();
    stop.name = proto_stop.name();
    stop.coord = CreateCoord(proto_stop.coord());

    return stop;
}

transport_catalogue::bus_catalogue::Bus CreateBus(const transport_proto::Bus& proto_bus, const request_handler::RequestHandler& request_handler) {
    transport_catalogue::bus_catalogue::Bus bus;

    bus.name = proto_bus.name();
    // std::cerr << bus.name << " route size is " << proto_bus.route_size() << std::endl;
    for (int i = 0; i < proto_bus.route_size(); ++i) {
        const transport_catalogue::stop_catalogue::Stop* stop = request_handler.GetStopById(proto_bus.route(i));
        // std::cerr << stop->id << ' ' << stop->name << std::endl;
        bus.route.push_back(stop);
    }
    bus.route_type = transport_catalogue::RouteTypeFromInt(proto_bus.type());
    bus.route_geo_length = proto_bus.route_geo_length();
    bus.route_true_length = proto_bus.route_true_length();
    bus.stops_on_route = proto_bus.stops_on_route();
    bus.unique_stops = proto_bus.unique_stops();

    return bus;
}

svg::Point CreatePoint(const transport_proto::Point& proto_point) {
    svg::Point point;

    point.x = proto_point.x();
    point.y = proto_point.y();

    return point;
}

svg::Color CreateColor(const transport_proto::Color& proto_color) {
    svg::Color color; // monostate by default

    svg::ColorType type = svg::ColorTypeFromInt(proto_color.type());
    if (type == svg::ColorType::STRING) {
        color = proto_color.name();
    }
    else if (type == svg::ColorType::RGB) {
        color = svg::Rgb::FromStringView(proto_color.name());
    }
    else if (type == svg::ColorType::RGBA) {
        color = svg::Rgba::FromStringView(proto_color.name());
    }

    return color;
}

map_renderer::MapRendererSettings CreateMapRenderSettings(const transport_proto::MapRenderSettings& proto_settings) {
    map_renderer::MapRendererSettings settings;

    settings.width = proto_settings.width();
    settings.height = proto_settings.height();
    settings.padding = proto_settings.padding();
    settings.line_width = proto_settings.line_width();
    settings.stop_radius = proto_settings.stop_radius();
    settings.bus_label_font_size = proto_settings.bus_label_font_size();
    settings.bus_label_offset = CreatePoint(proto_settings.bus_label_offset());
    settings.stop_label_font_size = proto_settings.stop_label_font_size();
    settings.stop_label_offset = CreatePoint(proto_settings.stop_label_offset());
    settings.underlayer_color = CreateColor(proto_settings.underlayer_color());
    settings.underlayer_width = proto_settings.underlayer_width();
    for (int i = 0; i < proto_settings.color_palette_size(); ++i) {
        settings.color_palette.push_back(CreateColor(proto_settings.color_palette(i)));
    }

    return settings;
}

transport_catalogue::RouteSettings CreateRouteSettings(const transport_proto::RouteSettings& proto_settings) {
    transport_catalogue::RouteSettings settings;

    settings.bus_velocity = proto_settings.bus_velocity();
    settings.bus_wait_time = proto_settings.bus_waiting_time();

    return settings;
}

void Deserialization(request_handler::RequestHandler& request_handler, std::ifstream& in) {
    transport_proto::TransportCatalogue tc;
    tc.ParseFromIstream(&in);
    
    for (int i = 0; i < tc.stop_size(); ++i) {
        const transport_proto::Stop& stop = tc.stop(i);
        std::string name = stop.name();
        Coordinates coord = CreateCoord(stop.coord());
        // std::cerr << stop.id() << ' ' << name << std::endl;
        request_handler.AddStop(stop.id(), std::move(name), std::move(coord));
    }

    // std::cerr << "Bus size is " << tc.bus_size() << std::endl;
    for (int i = 0; i < tc.bus_size(); ++i) {
        transport_catalogue::bus_catalogue::Bus bus = CreateBus(tc.bus(i), request_handler);
        request_handler.AddBus(std::move(bus));
    }

    map_renderer::MapRendererSettings map_renderer_settings = CreateMapRenderSettings(tc.map_render_setting());
    request_handler.RenderMap(std::move(map_renderer_settings));

    // transport_catalogue::RouteSettings route_settings = CreateRouteSettings(tc.route_settings());
    // request_handler.SetRouteSettings(std::move(route_settings));
}

} // namespace transport_serialization
