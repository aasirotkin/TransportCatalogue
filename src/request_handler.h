#pragma once

#include "json.h"
#include "json_builder.h"
#include "geo.h"
#include "map_renderer.h"
#include "transport_catalogue.h"

#include <istream>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

namespace request_handler {

// ---------- RequestHandler --------------------------------------------------

class RequestHandler {
public:
    RequestHandler(transport_catalogue::TransportCatalogue& catalogue);

    // Метод добавляет новую остановку
    void AddStop(std::string&& name, Coordinates&& coord);

    // Метод добавляет реальную дистанцию между двумя остановками
    void AddDistance(std::string_view name_from, std::string_view name_to, double distance);

    // Метод добавляет новый маршрут
    void AddBus(std::string&& name, std::vector<std::string_view>&& route, transport_catalogue::RouteType type);

    // Метод инициализирует переменную с значением карты маршрутов в svg формате
    void RenderMap(map_renderer::MapRendererSettings&& settings);

    // Метод возвращает массив автобусов, проходящих через заданную остановку
    const std::pair<std::set<std::string_view>, bool> GetStopBuses(std::string_view name) const {
        return catalogue_.GetBusesForStop(name);
    }

    // Метод возвращает заданный маршрут
    const transport_catalogue::detail::VirtualPair<transport_catalogue::bus_catalogue::Bus> GetBus(std::string_view name) const {
        return catalogue_.GetBus(name);
    }

    // Метод возвращает инициализированную ранее переменную с картой маршрутов в svg формате
    const std::optional<std::string>& GetMap() const {
        return map_renderer_value_;
    }

private:
    transport_catalogue::TransportCatalogue& catalogue_;
    std::optional<std::string> map_renderer_value_;
};

// ----------------------------------------------------------------------------

namespace detail_base {

// Функция обрабатывает запрос на создание остановки
void RequestBaseStopProcess(
    RequestHandler& request_handler,
    const json::Node* node);

// Функция обрабатывает запрос на создание автобусного маршрута
void RequestBaseBusProcess(
    RequestHandler& request_handler,
    const json::Node* node);

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

} // namespace request_handler
