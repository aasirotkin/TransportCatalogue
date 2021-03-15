#pragma once

#include "domain.h"
#include "router.h"
#include "transport_catalogue.h"

#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace transport_graph {

using namespace transport_catalogue;

using TransportTime = double;

class TransportGraph : public graph::DirectedWeightedGraph<TransportTime> {
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

    const std::unordered_map<const stop_catalogue::Stop*, graph::VertexId>& GetStopToVertexIdExit() const {
        return stop_to_vertex_id_exit_;
    }

private:
    using EdgesData = std::unordered_map<graph::VertexId, std::unordered_map<graph::VertexId, TransportGraphData>>;

    static constexpr double TO_MINUTES = (3.6 / 60.0);

private:
    void InitVertexId(const TransportCatalogue& catalogue);

    void CreateGraph(const TransportCatalogue& catalogue);

    template <typename ConstIterator>
    void UpdateEdges(EdgesData& edges, const ConstIterator& begin, const ConstIterator& end, const bus_catalogue::Bus* bus_ptr, const TransportCatalogue& catalogue);

    void AddEdgesToGraph(const EdgesData& edges);

private:
    std::unordered_map<graph::EdgeId, TransportGraphData> edge_id_to_graph_data_;

    std::unordered_map<const stop_catalogue::Stop*, graph::VertexId> stop_to_vertex_id_;
    std::unordered_map<const stop_catalogue::Stop*, graph::VertexId> stop_to_vertex_id_exit_;
};

template<typename ConstIterator>
inline void TransportGraph::UpdateEdges(EdgesData& edges, const ConstIterator& begin, const ConstIterator& end, const bus_catalogue::Bus* bus_ptr, const TransportCatalogue& catalogue) {
    const auto& stop_distances = catalogue.GetStopsCatalogue().GetDistances();
    const double bus_velocity = catalogue.GetBusCatalogue().GetRouteSettings().bus_velocity;

    for (auto it_from = begin; it_from != end; ++it_from) {
        const stop_catalogue::Stop* stop_from = *it_from;
        const stop_catalogue::Stop* previous_stop = stop_from;
        double full_distance = 0.0;
        int stop_count = 0;

        graph::VertexId from = stop_to_vertex_id_.at(stop_from);
        if (edges.count(from) == 0) {
            edges[from] = {};
        }

        for (auto it_to = it_from + 1; it_to != end; ++it_to) {
            const stop_catalogue::Stop* stop_to = *it_to;
            if (stop_from == stop_to) {
                previous_stop = stop_to;
                continue;
            }

            stop_count++;
            full_distance += stop_distances.at({ previous_stop, stop_to });
            previous_stop = stop_to;

            const double time = (full_distance / bus_velocity) * TO_MINUTES;

            graph::VertexId to = stop_to_vertex_id_exit_.at(stop_to);

            TransportGraphData data{ stop_from, stop_to, bus_ptr, stop_count, time };

            if (edges.at(from).count(to) > 0) {
                if (edges.at(from).at(to).time > time) {
                    edges.at(from).at(to) = data;
                }
                continue;
            }

            edges[from].insert({ to, data });
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

    std::optional<TransportRouterData> GetRoute(const stop_catalogue::Stop* from, const stop_catalogue::Stop* to) const;

private:
    const TransportGraph transport_graph_;
};

} // namespace transport_graph
