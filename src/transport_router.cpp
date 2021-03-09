#include "transport_router.h"

void transport_graph::TransportGraph::InitVertexId(const TransportCatalogue& catalogue) {
    const auto& stops = catalogue.GetStops();

    graph::VertexId vertex_id{};
    for (const auto& [stop_name, stop_ptr] : stops) {
        vertex_id_to_stop_.insert({ vertex_id, stop_ptr });
        stop_to_vertex_id_.insert({ stop_ptr, vertex_id++ });

        vertex_id_to_stop_exit_.insert({ vertex_id, stop_ptr });
        stop_to_vertex_id_exit_.insert({ stop_ptr, vertex_id++ });
    }
}

void transport_graph::TransportGraph::CreateGraph(const TransportCatalogue& catalogue) {
    for (const auto& [bus_name, bus_ptr] : catalogue.GetBuses()) {
        CreateGraphForBus(bus_ptr->route.begin(), bus_ptr->route.end(), bus_ptr, catalogue);

        if (bus_ptr->route_type == RouteType::BackAndForth) {
            CreateGraphForBus(bus_ptr->route.rbegin(), bus_ptr->route.rend(), bus_ptr, catalogue);
        }
    }
}

transport_graph::TransportRouter::TransportRouterData transport_graph::TransportRouter::GetRoute(const stop_catalogue::Stop* from, const stop_catalogue::Stop* to) const {
    TransportRouterData output_data;
    const auto& stop_to_vertex_id_exit = transport_graph_.GetStopToVertexIdExit();
    auto route = BuildRoute(stop_to_vertex_id_exit.at(from), stop_to_vertex_id_exit.at(to));
    if (route) {
        output_data.time = (*route).weight;

        const auto& edge_id_to_graph_data = transport_graph_.GetEdgeIdToGraphData();
        for (graph::EdgeId id : (*route).edges) {
            output_data.route.push_back(edge_id_to_graph_data.at(id));
        }
    }
    return output_data;
}
