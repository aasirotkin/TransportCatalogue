#pragma once

#include "json.h"
#include "json_builder.h"
#include "geo.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <istream>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

namespace request_handler {

enum class ProgrammType {
    MAKE_BASE,
    PROCESS_REQUESTS,
    UNKNOWN
};

ProgrammType ParseProgrammType(int argc, const char** argv);

// ---------- RequestHandler --------------------------------------------------

class RequestHandler {
private:
    using RouteData = transport_graph::TransportRouter::TransportRouterData;

public:
    RequestHandler(transport_catalogue::TransportCatalogue& catalogue);

    // Метод добавляет новую остановку
    void AddStop(std::string&& name, Coordinates&& coord);

    void AddStop(size_t id, std::string&& name, Coordinates&& coord);

    // Метод добавляет реальную дистанцию между двумя остановками
    void AddDistance(std::string_view name_from, std::string_view name_to, double distance);

    // Метод добавляет новый маршрут
    void AddBus(transport_catalogue::bus_catalogue::BusHelper&& bus_helper);

    void AddBus(transport_catalogue::bus_catalogue::Bus&& bus) {
        catalogue_.AddBus(std::move(bus));
    }

    // Метод инициализирует переменную с значением карты маршрутов в svg формате
    void RenderMap(map_renderer::MapRendererSettings&& settings);

    // Метод проверяет правильность маршрута
    bool IsRouteValid(
        const transport_catalogue::stop_catalogue::Stop* from,
        const transport_catalogue::stop_catalogue::Stop* to,
        const transport_catalogue::bus_catalogue::Bus* bus,
        int span, double time) const;

    // Метод проверяет существует ли остановка с заданным именем
    bool DoesStopExist(std::string_view name) const {
        return catalogue_.GetStops().At(name) != std::nullopt;
    }

    // Метод устанавливает общие настройки маршрута
    void SetRouteSettings(transport_catalogue::RouteSettings&& settings) {
        catalogue_.SetBusRouteCommonSettings(std::move(settings));
    }

    // Метод возвращает массив автобусов, проходящих через заданную остановку
    const std::set<std::string_view>& GetStopBuses(std::string_view name) const {
        return catalogue_.GetBusesForStop(name);
    }

    // Метод возвращает заданный маршрут
    const std::optional<const transport_catalogue::bus_catalogue::Bus*> GetBus(std::string_view name) const {
        return catalogue_.GetBuses().At(name);
    }

    // Метод возвращает инициализированную ранее переменную с картой маршрутов в svg формате
    const std::optional<std::string>& GetMap() const {
        return map_renderer_value_;
    }

    // Метод возвращает настройки отображения карты маршрутов
    const std::optional<map_renderer::MapRendererSettings>& GetMapRenderSettings() const {
        return map_render_settings_;
    }

    // Метод возвращает данные маршрута от остановки from до остановки to
    std::optional<RouteData> GetRoute(std::string_view from, std::string_view to) const;

    // Метод возвращает все существующие остановки
    std::vector<const transport_catalogue::stop_catalogue::Stop*> GetStops() const {
        std::vector<const transport_catalogue::stop_catalogue::Stop*> stops;

        for (const auto& [name, stop] : catalogue_.GetStops()) {
            stops.push_back(stop);
        }

        return stops;
    }

    // Метод возвращает все существующие автобусные маршруты
    std::vector<const transport_catalogue::bus_catalogue::Bus*> GetBuses() const {
        std::vector<const transport_catalogue::bus_catalogue::Bus*> buses;

        for (const auto& [name, bus] : catalogue_.GetBuses()) {
            buses.push_back(bus);
        }

        return buses;
    }

    // Метод возвращает настройки маршрута
    const transport_catalogue::RouteSettings& GetRouteSettings() const {
        return catalogue_.GetBuses().GetRouteSettings();
    }

    // Метод возвращает указатель на остановку через уникальный номер
    const transport_catalogue::stop_catalogue::Stop* GetStopById(size_t id) const {
        return catalogue_.GetStops().GetStopById(id);
    }

private:
    transport_catalogue::TransportCatalogue& catalogue_;
    std::optional<std::string> map_renderer_value_;
    std::optional<map_renderer::MapRendererSettings> map_render_settings_;
    mutable std::unique_ptr<transport_graph::TransportGraph> graph_;
    mutable std::unique_ptr<transport_graph::TransportRouter> router_;
};

// ----------------------------------------------------------------------------

namespace detail_base {

// Функция обрабатывает запрос на создание остановки
void RequestBaseStopProcess(
    RequestHandler& request_handler,
    const json::Node* node);

// Функция обрабатывает запрос на создание автобусного маршрута
transport_catalogue::bus_catalogue::BusHelper RequestBaseBusProcess(const json::Node* node);

// Функция преобразует json узел в цвет
svg::Color ParseColor(const json::Node* node);

// Функция преобразует json массив в набор цветов
std::vector<svg::Color> ParsePaletteColors(const json::Array& array_color_palette);

// Функция преобразует json массив в координаты точки на проскости
svg::Point ParseOffset(const json::Array& offset);

// Функция обрабатывает запрос на создания карты маршрутов
void RequestBaseMapProcess(
    RequestHandler& request_handler,
    const std::unordered_map<std::string_view, const json::Node*>& render_settings);

} // namespace detail_base

// ----------------------------------------------------------------------------

namespace detail_stat {

// Функция обрабатывает запрос на получение информации об остановке
void RequestStatStopProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Dict& request);

// Функция обрабатывает запрос на получение информации о автобусе
void RequestStatBusProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Dict& request);

// Функция обрабатывает запрос на получение карты маршрутов
void RequestMapProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Dict& request);

// Функция обрабатывающая все запросы
void RequestStatProcess(
    json::Builder& builder,
    const RequestHandler& request_handler,
    const json::Node* node);

} // namespace detail_stat

// ----------------------------------------------------------------------------

void RequestHandlerProcess(std::istream& input, std::ostream& output);

void RequestHandlerMakeBaseProcess(std::istream& input);

void RequestHandlerProcessRequestProcess(std::istream& input, std::ostream& output);

} // namespace request_handler
