#pragma once

#include "domain.h"
#include "route.h"

#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <unordered_map>
#include <vector>

namespace transport_catalogue {

// ----------------------------------------------------------------------------

class TransportCatalogue {
public:
    TransportCatalogue() = default;

    void AddBus(bus_catalogue::Bus&& add_bus);

    void AddStop(std::string&& name, std::string&& string_coord);

    void AddStop(std::string&& name, Coordinates&& coord);

    void AddDistanceBetweenStops(const std::string_view& stop_from_name, const std::string_view& stop_to_name, double distance);

    detail::VirtualPair<bus_catalogue::Bus> GetBus(const std::string_view& name) const;

    const stop_catalogue::BusesToStopNames& GetBusesForStop(const std::string_view& name) const;

    const detail::VirtualCatalogue<stop_catalogue::Stop>& GetStops() const;

    const detail::VirtualCatalogue<bus_catalogue::Bus>& GetBuses() const;

    const stop_catalogue::Catalogue& GetStopsCatalogue() const;

private:
    bus_catalogue::Catalogue buses_;
    detail::VirtualCatalogue<bus_catalogue::Bus> virtual_buses_;

    stop_catalogue::Catalogue stops_;
    detail::VirtualCatalogue<stop_catalogue::Stop> virtual_stops_;
};

// ----------------------------------------------------------------------------

namespace transport_graph {

using TransportTime = double;

class TransportGraph : public graph::DirectedWeightedGraph<TransportTime> {
private:
    static constexpr double TO_MINUTES = (3.6 / 60.0);

public:
    struct TransportGraphData {
        const stop_catalogue::Stop* from;
        const stop_catalogue::Stop* to;
        const bus_catalogue::Bus* bus;
    };

public:
    explicit TransportGraph(const TransportCatalogue& catalogue)
        : graph::DirectedWeightedGraph<TransportTime>(3 * catalogue.GetStopsCatalogue().Size()) {
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
    void InitVertexId(const TransportCatalogue& catalogue) {
        const auto& stops = catalogue.GetStops();

        graph::VertexId vertex_id{};
        for (const auto& [stop_name, stop_ptr] : stops) {
            vertex_id_to_stop_.insert({ vertex_id, stop_ptr });
            stop_to_vertex_id_.insert({ stop_ptr, vertex_id++ });

            vertex_id_to_stop_exit_.insert({ vertex_id, stop_ptr });
            stop_to_vertex_id_exit_.insert({ stop_ptr, vertex_id++ });
        }
    }

    void CreateGraph(const TransportCatalogue& catalogue) {
        const auto& stop_distances = catalogue.GetStopsCatalogue().GetDistances();

        for (const auto& [bus_name, bus_ptr] : catalogue.GetBuses()) {
            for (auto it_from = bus_ptr->route.begin(); it_from != bus_ptr->route.end(); ++it_from) {
                const stop_catalogue::Stop* stop_from = *it_from;
                const stop_catalogue::Stop* previous_stop = stop_from;
                double full_distance = 0.0;

                for (auto it_to = it_from; it_to != bus_ptr->route.end(); ++it_to) {
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
                        from = stop_to_vertex_id_.at(stop_from);
                        to = stop_to_vertex_id_exit_.at(stop_to);
                        full_distance += stop_distances.at({ previous_stop, stop_to });
                        time = (full_distance / bus_ptr->route_settings.bus_velocity) * TO_MINUTES;
                    }

                    previous_stop = stop_to;

                    graph::EdgeId id = AddEdge({ from, to, time });

                    edge_id_to_graph_data_.insert({ id, { stop_from, stop_to, bus_ptr } });
                }
            }
        }
    }

private:
    std::unordered_map<graph::EdgeId, TransportGraphData> edge_id_to_graph_data_;

    std::unordered_map<const stop_catalogue::Stop*, graph::VertexId> stop_to_vertex_id_;
    std::unordered_map<graph::VertexId, const stop_catalogue::Stop*> vertex_id_to_stop_;

    std::unordered_map<const stop_catalogue::Stop*, graph::VertexId> stop_to_vertex_id_exit_;
    std::unordered_map<graph::VertexId, const stop_catalogue::Stop*> vertex_id_to_stop_exit_;
};

// ----------------------------------------------------------------------------

class TransportRouter : private graph::Router<TransportTime> {
public:
    struct TransportRouterData {
        std::vector<TransportGraph::TransportGraphData> route{};
        TransportTime time{};
    };

public:
    TransportRouter(const TransportGraph& transport_graph)
        : transport_graph_(transport_graph)
        , graph::Router<TransportTime>(transport_graph) {
    }

    TransportRouterData GetRoute(const stop_catalogue::Stop* from, const stop_catalogue::Stop* to) const {
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

private:
    const TransportGraph transport_graph_;
};

} // namespace transport_graph

// ----------------------------------------------------------------------------

} // namespace transport_catalogue
