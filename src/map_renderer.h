#pragma once

#include <algorithm>
#include <deque>
#include <iostream>
#include <map>
#include <set>
#include <string_view>

#include "domain.h"
#include "geo.h"
#include "svg.h"

struct MapRendererSettings {
    MapRendererSettings() = default;

    // width ����� ������ ������� ����� � ��������
    // ������������ ����� � ��������� �� 0 �� 100000
    double width = 0.0;

    // height ����� ������ ������� ����� � ��������
    // ������������ ����� � ��������� �� 0 �� 100000.
    double height = 0.0;

    // ������ ���� ����� �� ������ SVG - ���������
    // ������������ ����� �� ������ 0 � ������ min(width, height) / 2
    double padding = 0.0;

    // ������� �����, �������� �������� ���������� ��������
    // ������������ ����� � ��������� �� 0 �� 100000
    double line_width = 0.0;

    // ������ �����������, �������� ������������ ���������
    // ������������ ����� � ��������� �� 0 �� 100000
    double stop_radius = 0.0;

    // ����� �����, �������� ������ ������ ������, ������� ������������ �������� ���������� ���������
    // ����� ����� � ��������� �� 0 �� 100000
    int bus_label_font_size = 0;

    // �������� ������� � ��������� ����������� �������� ������������ ��������� �������� ��������� �� �����
    // ����� �������� ������� dx � dy SVG - �������� <text>
    // ������ �� ���� ��������� ���� double, �������� � ��������� �� �100000 �� 100000
    svg::Point bus_label_offset = {};

    // ������ ������ ������, ������� ������������ �������� ���������
    // ����� ����� � ��������� �� 0 �� 100000.
    int stop_label_font_size = 0;

    // �������� ������� � ��������� ��������� ������������ ��������� ��������� �� �����
    // ����� �������� ������� dx � dy SVG - �������� <text>
    // ������ �� ���� ��������� ���� double, �������� � ��������� �� �100000 �� 100000
    svg::Point stop_label_offset = {};

    // ���� ��������, ������������ ��� ���������� ��������� � ���������� ���������
    svg::Color underlayer_color = {};

    // ������� �������� ��� ���������� ��������� � ���������� ���������
    // ����� �������� �������� stroke - width �������� <text>
    // ������������ ����� � ��������� �� 0 �� 100000
    double underlayer_width = 0.0;

    // �������� �������, ������������ ��� ������������ ���������
    std::vector<svg::Color> color_palette = {};
};

class MapRenderer : public svg::Document {
public:
    MapRenderer(
        MapRendererSettings&& render_settings,
        const transport_catalogue::stop_catalogue::Catalogue& stop_catalogue,
        const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::stop_catalogue::Stop>& stops,
        const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::bus_catalogue::Bus>& buses);

private:
    // ������������� ������� ���������, ����� ������� �������� ���� �� ���� �������
    void InitNotEmptyStops(
        const transport_catalogue::stop_catalogue::Catalogue& catalogue,
        const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::stop_catalogue::Stop>& stops);

    // ������������� ������� ���������, ������� ����� ���� �� ���� ���������
    void InitNotEmptyBuses(
        const transport_catalogue::detail::VirtualCatalogue<transport_catalogue::bus_catalogue::Bus>& buses);

    // ���������� �������� ����������� ���������������
    void CalculateZoomCoef();

    // ���������� ��������� ��������� � ������ ������������ ���������������
    void CalculateStopZoomedCoords();

    // ��������� ����� ���������
    void DrawLines();

    // ��������� �������� ���������
    void DrawBusText();

    // ��������� ������ ���������
    void DrawStopCircles();

    // ��������� �������� ���������
    void DrawStopText();

private:
    MapRendererSettings render_settings_;
    double zoom_coef_ = 0.0;
    double min_longitude_ = 0.0;
    double max_latitude_ = 0.0;

    std::map<const transport_catalogue::stop_catalogue::Stop*, svg::Point> stop_point_;
    std::map<std::string_view, const transport_catalogue::stop_catalogue::Stop*> stops_;
    std::map<std::string_view, const transport_catalogue::bus_catalogue::Bus*> buses_;
};
