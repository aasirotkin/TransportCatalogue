#include "serialization.h"

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

    proto_color.set_name(svg::GetColorStringName(color));

    return proto_color;
}

transport_proto::MapRenderSettings CreateProtoRenderSettings(const map_renderer::MapRendererSettings& setting) {
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

    

    (void)request_handler;

    tc.SerializeToOstream(&out);
}

void Deserialization(request_handler::RequestHandler& request_handler, std::ifstream& in) {
    transport_proto::TransportCatalogue tc;
    tc.ParseFromIstream(&in);
    (void)request_handler;
}

} // namespace transport_serialization
