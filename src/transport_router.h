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

    struct VertexIdLoop {
        graph::VertexId id{};
        graph::VertexId transfer_id{};
    };

public:
    explicit TransportGraph(const TransportCatalogue& catalogue)
        : graph::DirectedWeightedGraph<TransportTime>(2 * catalogue.GetStops().Size()) {
        InitVertexId(catalogue);
        CreateDiagonalEdges(catalogue);
        CreateGraph(catalogue);
    }

    const std::unordered_map<graph::EdgeId, TransportGraphData>& GetEdgeIdToGraphData() const {
        return edge_id_to_graph_data_;
    }

    const std::unordered_map<const stop_catalogue::Stop*, VertexIdLoop>& GetStopToVertexId() const {
        return stop_to_vertex_id_;
    }

private:
    static constexpr double TO_MINUTES = (3.6 / 60.0);

    using EdgesData = std::unordered_map<graph::VertexId, std::unordered_map<graph::VertexId, TransportGraphData>>;

private:
    void InitVertexId(const TransportCatalogue& catalogue);

    void CreateDiagonalEdges(const TransportCatalogue& catalogue);

    void CreateGraph(const TransportCatalogue& catalogue);

    template <typename It>
    std::vector<TransportGraphData> CreateTransportGraphData(ranges::BusRange<It> bus_range, const TransportCatalogue& catalogue);

    void CreateEdges(EdgesData& edges, std::vector<TransportGraphData>&& data);

    void AddEdgesToGraph(EdgesData& edges);

private:
    std::unordered_map<graph::EdgeId, TransportGraphData> edge_id_to_graph_data_;

    std::unordered_map<const stop_catalogue::Stop*, VertexIdLoop> stop_to_vertex_id_;
};

template <typename It>
inline std::vector<TransportGraph::TransportGraphData> TransportGraph::CreateTransportGraphData(ranges::BusRange<It> bus_range, const TransportCatalogue& catalogue) {
    const auto& stop_distances = catalogue.GetStops().GetDistances();
    const double bus_velocity = catalogue.GetBuses().GetRouteSettings().bus_velocity;

    std::vector<TransportGraph::TransportGraphData> data;

    for (auto it_from = bus_range.begin(); it_from != bus_range.end(); ++it_from) {
        const stop_catalogue::Stop* stop_from = *it_from;
        const stop_catalogue::Stop* previous_stop = stop_from;

        double full_distance = 0.0;
        int stop_count = 0;

        for (auto it_to = it_from + 1; it_to != bus_range.end(); ++it_to) {
            const stop_catalogue::Stop* stop_to = *it_to;

            if (stop_from != stop_to) {
                full_distance += stop_distances.at({ previous_stop, stop_to });
                stop_count++;

                data.push_back({ stop_from, stop_to, bus_range.GetPtr(), stop_count, (full_distance / bus_velocity) * TO_MINUTES });
            }

            previous_stop = stop_to;
        }
    }

    return data;
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
