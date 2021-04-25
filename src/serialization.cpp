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

transport_proto::Stop CreateProtoStop(const transport_catalogue::stop_catalogue::Stop* stop, const request_handler::RequestHandler& rh) {
    transport_proto::Stop proto_stop;

    proto_stop.set_id(rh.GetId(stop));
    proto_stop.set_name(stop->name);
    *proto_stop.mutable_coord() = CreateProtoCoord(stop->coord);

    return proto_stop;
}

transport_proto::Bus CreateProtoBus(const transport_catalogue::bus_catalogue::Bus* bus, const request_handler::RequestHandler& rh) {
    transport_proto::Bus proto_bus;

    proto_bus.set_name(bus->name);
    for (const auto* stop : bus->route) {
        proto_bus.add_route(rh.GetId(stop));
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

transport_proto::EdgeId CreateProtoEdgeId(graph::EdgeId id) {
    transport_proto::EdgeId proto_edge_id;

    proto_edge_id.set_id(id);

    return proto_edge_id;
}

transport_proto::VertexId CreateProtoVertexId(graph::VertexId id) {
    transport_proto::VertexId proto_vertex_id;

    proto_vertex_id.set_id(id);

    return proto_vertex_id;
}

transport_proto::Weight CreateProtoWeight(double weight) {
    transport_proto::Weight proto_weight;

    proto_weight.set_value(weight);

    return proto_weight;
}

transport_proto::Edge CreateProtoEdge(const graph::Edge<double>& edge) {
    transport_proto::Edge proto_edge;

    //std::cerr << "Serializing mutable_from" << std::endl;
    *proto_edge.mutable_from() = CreateProtoVertexId(edge.from);
    //std::cerr << "Serializing mutable_to" << std::endl;
    *proto_edge.mutable_to() = CreateProtoVertexId(edge.to);
    //std::cerr << "Serializing mutable_weight" << std::endl;
    *proto_edge.mutable_weight() = CreateProtoWeight(edge.weight);

    return proto_edge;
}

transport_proto::IncidenceList CreateProtoIncidenceList(const graph::DirectedWeightedGraph<double>::IncidenceList& incidence_list) {
    transport_proto::IncidenceList proto_incidence_list;

    for (const auto& edge_id : incidence_list) {
        *proto_incidence_list.add_id() = CreateProtoEdgeId(edge_id);
    }

    return proto_incidence_list;
}

transport_proto::TransportGraphData CreateProtoTransportGraphData(const transport_graph::TransportGraph::TransportGraphData& data, const request_handler::RequestHandler& rh) {
    transport_proto::TransportGraphData proto_data;

    proto_data.set_stop_to_id(rh.GetId(data.to));
    proto_data.set_stop_from_id(rh.GetId(data.from));
    if (data.bus) {
        proto_data.set_bus_id(rh.GetId(data.bus));
    }
    proto_data.set_stop_count(data.stop_count);
    proto_data.set_time(data.time);

    return proto_data;
}

transport_proto::Graph CreateProtoGraph(const transport_graph::TransportGraph& graph, const request_handler::RequestHandler& rh) {
    transport_proto::Graph proto_graph;

    //std::cerr << "Serializing edges" << std::endl;
    for (const auto& edge : graph.GetEdges()) {
        *proto_graph.add_edge() = CreateProtoEdge(edge);
    }

    //std::cerr << "Serializing incidence_list" << std::endl;
    for (const auto& incidence_list : graph.GetIncidenceList()) {
        *proto_graph.add_incidence_list() = CreateProtoIncidenceList(incidence_list);
    }

    //std::cerr << "Serializing graph_data" << std::endl;
    for (const auto& [edge_id, graph_data] : graph.GetEdgeIdToGraphData()) {
        (*proto_graph.mutable_edge_id_to_graph_data())[static_cast<uint32_t>(edge_id)] = CreateProtoTransportGraphData(graph_data, rh);
    }

    return proto_graph;
}

void Serialization(std::ofstream& out, const request_handler::RequestHandler& rh) {
    transport_proto::TransportCatalogue tc;

    for (const transport_catalogue::stop_catalogue::Stop* stop : rh.GetStops()) {
        *tc.add_stop() = CreateProtoStop(stop, rh);
    }

    for (const transport_catalogue::bus_catalogue::Bus* bus : rh.GetBuses()) {
        *tc.add_bus() = CreateProtoBus(bus, rh);
    }

    const auto& map_render_settings = rh.GetMapRenderSettings();
    if (map_render_settings) {
        *tc.mutable_map_render_setting() = CreateProtoMapRenderSettings(map_render_settings.value());
    }

    *tc.mutable_route_settings() = CreateProtoRouteSetting(rh.GetRouteSettings());

    if (rh.GetGraph()) {
        *tc.mutable_graph() = CreateProtoGraph(*rh.GetGraph(), rh);
    }

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

    stop.name = proto_stop.name();
    stop.coord = CreateCoord(proto_stop.coord());

    return stop;
}

transport_catalogue::bus_catalogue::Bus CreateBus(const transport_proto::Bus& proto_bus, const request_handler::RequestHandler& rh) {
    transport_catalogue::bus_catalogue::Bus bus;

    bus.name = proto_bus.name();
    for (int i = 0; i < proto_bus.route_size(); ++i) {
        const transport_catalogue::stop_catalogue::Stop* stop = rh.GetStopById(proto_bus.route(i));
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

//transport_graph::TransportGraph CreateGraph(const transport_proto::Graph& proto_graph) {
//    transport_graph::TransportGraph graph;
//    (void)proto_graph;
//    return graph;
//}

void Deserialization(request_handler::RequestHandler& rh, std::ifstream& in) {
    transport_proto::TransportCatalogue tc;
    tc.ParseFromIstream(&in);

    for (int i = 0; i < tc.stop_size(); ++i) {
        const transport_proto::Stop& stop = tc.stop(i);
        rh.AddStop(stop.id(), CreateStop(stop));
    }

    for (int i = 0; i < tc.bus_size(); ++i) {
        const transport_proto::Bus& bus = tc.bus(i);
        rh.AddBus(bus.id(), CreateBus(bus, rh));
    }

    rh.RenderMap(CreateMapRenderSettings(tc.map_render_setting()));

    //rh.SetRouteSettings(std::move(CreateRouteSettings(tc.route_settings())));

    //request_handler.SetGraph(std::move(CreateGraph(tc.graph())));
}

} // namespace transport_serialization
