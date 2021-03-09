#pragma once

#include "domain.h"
#include "router.h"
#include "transport_catalogue.h"

#include <unordered_map>

namespace transport_graph {

using namespace transport_catalogue;

using TransportTime = double;

class TransportGraph : public graph::DirectedWeightedGraph<TransportTime> {
private:
    static constexpr double TO_MINUTES = (3.6 / 60.0);

public:
    struct TransportGraphData {
        const stop_catalogue::Stop* from;
        const stop_catalogue::Stop* to;
        const bus_catalogue::Bus* bus;
        int stop_count;
        double time;
    };

public:
    explicit TransportGraph(const TransportCatalogue& catalogue)
        : graph::DirectedWeightedGraph<TransportTime>(2 * catalogue.GetStopsCatalogue().Size()) {
        InitVertexId(catalogue);
        CreateGraph(catalogue);
    }

    const std::unordered_map<graph::EdgeId, TransportGraphData>& GetEdgeIdToGraphData() const {
        return edge_id_to_graph_data_;
    }

    const std::unordered_map<const stop_catalogue::Stop*, graph::VertexId>& GetStopToVertexId() const {
        return stop_to_vertex_id_;
    }

    const std::unordered_map<graph::VertexId, const stop_catalogue::Stop*>& GetVertexIdToStop() const {
        return vertex_id_to_stop_;
    }

    const std::unordered_map<const stop_catalogue::Stop*, graph::VertexId>& GetStopToVertexIdExit() const {
        return stop_to_vertex_id_exit_;
    }

    const std::unordered_map<graph::VertexId, const stop_catalogue::Stop*>& GetVertexIdToStopExit() const {
        return vertex_id_to_stop_exit_;
    }

private:
    void InitVertexId(const TransportCatalogue& catalogue);

    void CreateGraph(const TransportCatalogue& catalogue);

    template <typename ConstIterator>
    void CreateGraphForBus(ConstIterator begin, ConstIterator end, const bus_catalogue::Bus* bus_ptr, const TransportCatalogue& catalogue);

private:
    std::unordered_map<graph::EdgeId, TransportGraphData> edge_id_to_graph_data_;

    std::unordered_map<const stop_catalogue::Stop*, graph::VertexId> stop_to_vertex_id_;
    std::unordered_map<graph::VertexId, const stop_catalogue::Stop*> vertex_id_to_stop_;

    std::unordered_map<const stop_catalogue::Stop*, graph::VertexId> stop_to_vertex_id_exit_;
    std::unordered_map<graph::VertexId, const stop_catalogue::Stop*> vertex_id_to_stop_exit_;
};

template<typename ConstIterator>
inline void TransportGraph::CreateGraphForBus(ConstIterator begin, ConstIterator end, const bus_catalogue::Bus* bus_ptr, const TransportCatalogue& catalogue) {
    const auto& stop_distances = catalogue.GetStopsCatalogue().GetDistances();
    for (auto it_from = begin; it_from != end; ++it_from) {
        const stop_catalogue::Stop* stop_from = *it_from;
        const stop_catalogue::Stop* previous_stop = stop_from;
        double full_distance = 0.0;
        int stop_count = 0;

        for (auto it_to = it_from; it_to != end; ++it_to) {
            const stop_catalogue::Stop* stop_to = *it_to;
            graph::VertexId from{};
            graph::VertexId to{};
            double time{};

            if (stop_from == stop_to) {
                from = stop_to_vertex_id_exit_.at(stop_from);
                to = stop_to_vertex_id_.at(stop_to);
                time = bus_ptr->route_settings.bus_wait_time;
            }
            else {
                stop_count++;
                from = stop_to_vertex_id_.at(stop_from);
                to = stop_to_vertex_id_exit_.at(stop_to);
                full_distance += stop_distances.at({ previous_stop, stop_to });
                time = (full_distance / bus_ptr->route_settings.bus_velocity) * TO_MINUTES;
            }

            previous_stop = stop_to;

            graph::EdgeId id = AddEdge({ from, to, time });
            edge_id_to_graph_data_.insert({ id, { stop_from, stop_to, bus_ptr, stop_count, time } });
        }
    }
}

// ----------------------------------------------------------------------------

class TransportRouter : private graph::Router<TransportTime> {
public:
    struct TransportRouterData {
        std::vector<TransportGraph::TransportGraphData> route{};
        TransportTime time{};
    };

public:
    explicit TransportRouter(const TransportGraph& transport_graph)
        : graph::Router<TransportTime>(transport_graph)
        , transport_graph_(transport_graph) {
    }

    TransportRouterData GetRoute(const stop_catalogue::Stop* from, const stop_catalogue::Stop* to) const;

private:
    const TransportGraph transport_graph_;
};

} // namespace transport_graph