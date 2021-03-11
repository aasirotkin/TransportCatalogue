#include "transport_router.h"

namespace transport_graph {

void TransportGraph::InitVertexId(const TransportCatalogue& catalogue) {
    const auto& stops = catalogue.GetStops();
    const double time = static_cast<double>(catalogue.GetBusCatalogue().GetRouteSettings().bus_wait_time);

    graph::VertexId vertex_id{};
    for (const auto& [stop_name, stop_ptr] : stops) {
        stop_to_vertex_id_.insert({ stop_ptr, vertex_id });
        stop_to_vertex_id_exit_.insert({ stop_ptr, vertex_id + 1 });

        graph::EdgeId id = AddEdge({ vertex_id + 1, vertex_id, time });

        edge_id_to_graph_data_.insert({ id, { stop_ptr, stop_ptr, nullptr, 0, time } });

        vertex_id += 2;
    }
}

void TransportGraph::CreateGraph(const TransportCatalogue& catalogue) {
    std::unordered_map<graph::VertexId, std::unordered_map<graph::VertexId, TransportGraphData>> edges;

    for (const auto& [bus_name, bus_ptr] : catalogue.GetBuses()) {
        UpdateEdges(edges, bus_ptr->route.begin(), bus_ptr->route.end(), bus_ptr, catalogue);

        if (bus_ptr->route_type == RouteType::BackAndForth) {
            UpdateEdges(edges, bus_ptr->route.rbegin(), bus_ptr->route.rend(), bus_ptr, catalogue);
        }
    }

    AddEdgesToGraph(edges);
}

void TransportGraph::AddEdgesToGraph(const EdgesData& edges) {
    for (const auto& [from, to_map] : edges) {
        for (const auto& [to, data] : to_map) {
            graph::EdgeId id = AddEdge({ from, to, data.time });
            edge_id_to_graph_data_.insert({ id, data });
        }
    }
}

std::optional<TransportRouter::TransportRouterData> TransportRouter::GetRoute(const stop_catalogue::Stop* from, const stop_catalogue::Stop* to) const {
    const auto& stop_to_vertex_id_exit = transport_graph_.GetStopToVertexIdExit();
    auto route = BuildRoute(stop_to_vertex_id_exit.at(from), stop_to_vertex_id_exit.at(to));
    if (route) {
        TransportRouterData output_data;
        output_data.time = (*route).weight;

        const auto& edge_id_to_graph_data = transport_graph_.GetEdgeIdToGraphData();
        for (graph::EdgeId id : (*route).edges) {
            output_data.route.push_back(edge_id_to_graph_data.at(id));
        }
        return output_data;
    }
    return std::nullopt;
}

} // namespace transport_graph
