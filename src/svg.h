#pragma once

#include <cstdint>
#include <deque>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>

namespace svg {

// ---------- Point -----------------------------------------------------------

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    std::string Str() const;
    double x = 0.0;
    double y = 0.0;
};

// ---------- RenderContext ---------------------------------------------------

/*
* ��������������� ���������, �������� �������� ��� ������ SVG-��������� � ���������.
* ������ ������ �� ����� ������, ������� �������� � ��� ������� ��� ������ ��������
*/
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return { out, indent_step, indent + indent_step };
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

// ---------- Object ----------------------------------------------------------

/*
* ����������� ������� ����� Object ������ ��� ���������������� ��������
* ���������� ����� SVG-���������
* ��������� ������� "��������� �����" ��� ������ ����������� ����
*/
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

// ---------- Rgb -------------------------------------------------------------

#define TO_INT(c) (static_cast<int>(c))

struct Rgb {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    Rgb() = default;

    Rgb(uint8_t red, uint8_t green, uint8_t blue)
        : red(red)
        , green(green)
        , blue(blue) {
    }
};

inline std::ostream& operator<< (std::ostream& out, const Rgb& rgb) {
    using namespace std::literals;
    out << "rgb("sv << TO_INT(rgb.red) << ","sv << TO_INT(rgb.green) << ","sv << TO_INT(rgb.blue) << ")"sv;
    return out;
}

// ---------- Rgba ------------------------------------------------------------

struct Rgba : public Rgb {
    double opacity = 1.0;

    Rgba() = default;

    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity)
        : Rgb(red, green, blue)
        , opacity(opacity) {
    }
};

inline std::ostream& operator<< (std::ostream& out, const Rgba& rgba) {
    using namespace std::literals;
    out << "rgba("sv << TO_INT(rgba.red) << ","sv << TO_INT(rgba.green) << ","sv << TO_INT(rgba.blue) << ","sv << rgba.opacity << ")"sv;
    return out;
}

// ---------- Color -----------------------------------------------------------

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

inline const std::string NoneColor = std::string("none");
inline const std::string EmptyStringColor = std::string("");

struct ColorPrinter {
    std::string_view operator()(std::monostate) const {
        out << NoneColor;
        return EmptyStringColor;
    }

    std::string_view operator()(const std::string& color) const {
        out << color;
        return EmptyStringColor;
    }

    std::string_view operator()(const Rgb& rgb) const {
        out << rgb;
        return EmptyStringColor;
    }

    std::string_view operator()(const Rgba& rgba) const {
        out << rgba;
        return EmptyStringColor;
    }

    std::ostream& out;
};

#define PRINT_COLOR(out, color) (std::visit(ColorPrinter{out}, color))

// ---------- PathProps -------------------------------------------------------

/*
* ����� �������� ��������, ����������� ����������� ������� � �������
* ��������� ������� "Curiously Recurring Template Pattern" (CRTP)
*/

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

inline const std::unordered_map<StrokeLineCap, std::string> GetStrokeLineCapName = {
    {StrokeLineCap::BUTT, std::string("butt")},
    {StrokeLineCap::ROUND, std::string("round")},
    {StrokeLineCap::SQUARE, std::string("square")}
};

inline std::ostream& operator<< (std::ostream& out, StrokeLineCap cap) {
    out << GetStrokeLineCapName.at(cap);
    return out;
}

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

inline const std::unordered_map<StrokeLineJoin, std::string> GetStrokeLineJoinName = {
    {StrokeLineJoin::ARCS, std::string("arcs")},
    {StrokeLineJoin::BEVEL, std::string("bevel")},
    {StrokeLineJoin::MITER, std::string("miter")},
    {StrokeLineJoin::MITER_CLIP, std::string("miter-clip")},
    {StrokeLineJoin::ROUND, std::string("round")}
};

inline std::ostream& operator<< (std::ostream& out, StrokeLineJoin join) {
    out << GetStrokeLineJoinName.at(join);
    return out;
}

template <typename Owner>
class PathProps {
public:
    // ����� ���� �������
    Owner& SetFillColor(Color color);

    // ����� ���� �������
    Owner& SetStrokeColor(Color color);

    // ����� ������� �����
    Owner& SetStrokeWidth(double width);

    // ����� ��� ����� ����� �����
    Owner& SetStrokeLineCap(StrokeLineCap line_cap);

    // ����� ��� ����� ���������� �����
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const;

private:
    Owner& AsOwner();

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};

template<typename Owner>
inline Owner& PathProps<Owner>::SetFillColor(Color color) {
    fill_color_ = std::move(color);
    return AsOwner();
}

template<typename Owner>
inline Owner& PathProps<Owner>::SetStrokeColor(Color color) {
    stroke_color_ = std::move(color);
    return AsOwner();
}

template<typename Owner>
inline Owner& PathProps<Owner>::SetStrokeWidth(double width) {
    stroke_width_ = width;
    return AsOwner();
}

template<typename Owner>
inline Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
    stroke_line_cap_ = line_cap;
    return AsOwner();
}

template<typename Owner>
inline Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
    stroke_line_join_ = line_join;
    return AsOwner();
}

template<typename Owner>
void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
    using namespace std::literals;

    if (fill_color_) {
        out << " fill=\""sv << PRINT_COLOR(out, *fill_color_) << "\""sv;
    }
    if (stroke_color_) {
        out << " stroke=\""sv << PRINT_COLOR(out, *stroke_color_) << "\""sv;
    }
    if (stroke_width_) {
        out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
    }
    if (stroke_line_cap_) {
        out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
    }
    if (stroke_line_join_) {
        out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
    }
}

template<typename Owner>
Owner& PathProps<Owner>::AsOwner() {
    // static_cast ��������� ����������� *this � Owner&,
    // ���� ����� Owner - ��������� PathProps
    return static_cast<Owner&>(*this);
}

// ---------- Circle ----------------------------------------------------------

/*
* ����� Circle ���������� ������� <circle> ��� ����������� �����
* https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
*/
class Circle final : public Object, public PathProps<Circle> {
public:
    // ����� ���������� ������ ����������
    Circle& SetCenter(Point center);

    // ����� ������ ����������
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

private:
    Point center_;
    double radius_ = 1.0;
};

// ---------- Polyline --------------------------------------------------------

/*
* ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
* https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
*/
class Polyline : public Object, public PathProps<Polyline> {
public:
    // ��������� ��������� ������� � ������� �����
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;

private:
    std::string points_;
};

// ---------- Text ------------------------------------------------------------

/*
* ����� Text ���������� ������� <text> ��� ����������� ������
* https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
*/
class Text : public Object, public PathProps<Text> {
public:
    // ����� ���������� ������� ����� (�������� x � y)
    Text& SetPosition(Point pos);

    // ����� �������� ������������ ������� ����� (�������� dx, dy)
    Text& SetOffset(Point offset);

    // ����� ������� ������ (������� font-size)
    Text& SetFontSize(uint32_t size);

    // ����� �������� ������ (������� font-family)
    Text& SetFontFamily(std::string font_family);

    // ����� ������� ������ (������� font-weight)
    Text& SetFontWeight(std::string font_weight);

    // ����� ��������� ���������� ������� (������������ ������ ���� text)
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

private:
    Point pos_;
    Point offset_;
    uint32_t size_ = 1;
    std::optional<std::string> font_family_;
    std::optional<std::string> font_weight_;
    std::string data_;
};

// ---------- ObjectContainer -------------------------------------------------

class ObjectContainer {
public:
    // ����� Add ��������� � svg-��������� ����� ������-��������� svg::Object
    // ������� �������������� "��������� �����"
    template <typename Obj>
    void Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }

    // ��������� � svg-��������� ������-��������� svg::Object
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

protected:
    ~ObjectContainer() = default;
};

// ---------- Document --------------------------------------------------------

class Document : public ObjectContainer {
    using Objects = std::deque<std::unique_ptr<Object>>;
public:
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // ������� � ostream svg-������������� ���������
    void Render(std::ostream& out) const;

    const Objects& GetObjects() const;

private:
    Objects objects_;
};

// ---------- Drawable --------------------------------------------------------

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

}  // namespace svg
