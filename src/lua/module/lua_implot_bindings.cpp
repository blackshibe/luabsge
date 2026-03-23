#include "lua_implot_bindings.h"
#include <string>
#include <tuple>
#include <vector>
#include <cmath>

// Circular buffer for realtime plotting
struct LuaScrollingBuffer {
    int MaxSize;
    int Offset;
    std::vector<double> Xs;
    std::vector<double> Ys;

    LuaScrollingBuffer(int max_size = 2000) : MaxSize(max_size), Offset(0) {
        Xs.reserve(MaxSize);
        Ys.reserve(MaxSize);
    }

    void AddPoint(double x, double y) {
        if ((int)Xs.size() < MaxSize) {
            Xs.push_back(x);
            Ys.push_back(y);
        } else {
            Xs[Offset] = x;
            Ys[Offset] = y;
            Offset = (Offset + 1) % MaxSize;
        }
    }

    void Erase() {
        Xs.clear();
        Ys.clear();
        Offset = 0;
    }

    int Size() const { return (int)Xs.size(); }
};

// Rolling buffer (wraps around a span)
struct LuaRollingBuffer {
    float Span;
    std::vector<double> Xs;
    std::vector<double> Ys;

    LuaRollingBuffer(float span = 10.0f) : Span(span) {
        Xs.reserve(2000);
        Ys.reserve(2000);
    }

    void AddPoint(double x, double y) {
        double xmod = fmod(x, (double)Span);
        if (!Xs.empty() && xmod < Xs.back()) {
            Xs.clear();
            Ys.clear();
        }
        Xs.push_back(xmod);
        Ys.push_back(y);
    }

    void Erase() {
        Xs.clear();
        Ys.clear();
    }

    int Size() const { return (int)Xs.size(); }
};

void lua_bsge_init_implot_bindings(sol::state &lua) {
    auto implot = lua["ImPlot"].get_or_create<sol::table>();

    // ScrollingBuffer usertype
    lua.new_usertype<LuaScrollingBuffer>("ScrollingBuffer",
        sol::constructors<LuaScrollingBuffer(), LuaScrollingBuffer(int)>(),
        "AddPoint", &LuaScrollingBuffer::AddPoint,
        "Erase", &LuaScrollingBuffer::Erase,
        "Size", &LuaScrollingBuffer::Size,
        "MaxSize", &LuaScrollingBuffer::MaxSize,
        "Offset", &LuaScrollingBuffer::Offset
    );

    // RollingBuffer usertype
    lua.new_usertype<LuaRollingBuffer>("RollingBuffer",
        sol::constructors<LuaRollingBuffer(), LuaRollingBuffer(float)>(),
        "AddPoint", &LuaRollingBuffer::AddPoint,
        "Erase", &LuaRollingBuffer::Erase,
        "Size", &LuaRollingBuffer::Size,
        "Span", &LuaRollingBuffer::Span
    );

    // Begin/End Plot
    implot["BeginPlot"] = sol::overload(
        [](const char* title_id) { return ImPlot::BeginPlot(title_id); },
        [](const char* title_id, float w, float h) { return ImPlot::BeginPlot(title_id, ImVec2(w, h)); },
        [](const char* title_id, float w, float h, int flags) { return ImPlot::BeginPlot(title_id, ImVec2(w, h), flags); }
    );
    implot["EndPlot"] = &ImPlot::EndPlot;

    // Begin/End Subplots
    implot["BeginSubplots"] = sol::overload(
        [](const char* title_id, int rows, int cols, float w, float h) {
            return ImPlot::BeginSubplots(title_id, rows, cols, ImVec2(w, h));
        },
        [](const char* title_id, int rows, int cols, float w, float h, int flags) {
            return ImPlot::BeginSubplots(title_id, rows, cols, ImVec2(w, h), flags);
        }
    );
    implot["EndSubplots"] = &ImPlot::EndSubplots;

    // Setup
    implot["SetupAxis"] = sol::overload(
        [](int axis) { ImPlot::SetupAxis(axis); },
        [](int axis, const char* label) { ImPlot::SetupAxis(axis, label); },
        [](int axis, const char* label, int flags) { ImPlot::SetupAxis(axis, label, flags); }
    );
    implot["SetupAxisLimits"] = sol::overload(
        [](int axis, double v_min, double v_max) { ImPlot::SetupAxisLimits(axis, v_min, v_max); },
        [](int axis, double v_min, double v_max, int cond) { ImPlot::SetupAxisLimits(axis, v_min, v_max, cond); }
    );
    implot["SetupAxisFormat"] = [](int axis, const char* fmt) { ImPlot::SetupAxisFormat(axis, fmt); };
    implot["SetupAxisScale"] = [](int axis, int scale) { ImPlot::SetupAxisScale(axis, scale); };
    implot["SetupAxisLimitsConstraints"] = [](int axis, double v_min, double v_max) {
        ImPlot::SetupAxisLimitsConstraints(axis, v_min, v_max);
    };
    implot["SetupAxisZoomConstraints"] = [](int axis, double z_min, double z_max) {
        ImPlot::SetupAxisZoomConstraints(axis, z_min, z_max);
    };
    implot["SetupAxes"] = sol::overload(
        [](const char* x_label, const char* y_label) { ImPlot::SetupAxes(x_label, y_label); },
        [](const char* x_label, const char* y_label, int x_flags, int y_flags) {
            ImPlot::SetupAxes(x_label, y_label, x_flags, y_flags);
        }
    );
    implot["SetupAxesLimits"] = sol::overload(
        [](double x_min, double x_max, double y_min, double y_max) {
            ImPlot::SetupAxesLimits(x_min, x_max, y_min, y_max);
        },
        [](double x_min, double x_max, double y_min, double y_max, int cond) {
            ImPlot::SetupAxesLimits(x_min, x_max, y_min, y_max, cond);
        }
    );
    implot["SetupLegend"] = sol::overload(
        [](int location) { ImPlot::SetupLegend(location); },
        [](int location, int flags) { ImPlot::SetupLegend(location, flags); }
    );
    implot["SetupMouseText"] = sol::overload(
        [](int location) { ImPlot::SetupMouseText(location); },
        [](int location, int flags) { ImPlot::SetupMouseText(location, flags); }
    );
    implot["SetupFinish"] = &ImPlot::SetupFinish;

    // SetNext
    implot["SetNextAxisLimits"] = sol::overload(
        [](int axis, double v_min, double v_max) { ImPlot::SetNextAxisLimits(axis, v_min, v_max); },
        [](int axis, double v_min, double v_max, int cond) { ImPlot::SetNextAxisLimits(axis, v_min, v_max, cond); }
    );
    implot["SetNextAxisToFit"] = [](int axis) { ImPlot::SetNextAxisToFit(axis); };
    implot["SetNextAxesLimits"] = sol::overload(
        [](double x_min, double x_max, double y_min, double y_max) {
            ImPlot::SetNextAxesLimits(x_min, x_max, y_min, y_max);
        },
        [](double x_min, double x_max, double y_min, double y_max, int cond) {
            ImPlot::SetNextAxesLimits(x_min, x_max, y_min, y_max, cond);
        }
    );
    implot["SetNextAxesToFit"] = &ImPlot::SetNextAxesToFit;

    // Plot Items - all take Lua tables for data
    implot["PlotLine"] = sol::overload(
        // PlotLine(label, values) - auto x from 0..n
        [](const char* label_id, sol::table values) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotLine(label_id, v.data(), (int)v.size());
        },
        // PlotLine(label, values, xscale, xstart)
        [](const char* label_id, sol::table values, double xscale, double xstart) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotLine(label_id, v.data(), (int)v.size(), xscale, xstart);
        },
        // PlotLine(label, xs, ys) - explicit x/y
        [](const char* label_id, sol::table xs, sol::table ys) {
            std::vector<double> xv, yv;
            for (size_t i = 1; i <= xs.size(); i++) xv.push_back(xs[i]);
            for (size_t i = 1; i <= ys.size(); i++) yv.push_back(ys[i]);
            int count = (int)std::min(xv.size(), yv.size());
            ImPlot::PlotLine(label_id, xv.data(), yv.data(), count);
        }
    );

    // PlotLineStyled(label, xs, ys, r, g, b, a) - explicit x/y with color
    implot["PlotLineStyled"] = [](const char* label_id, sol::table xs, sol::table ys, float r, float g, float b, float a) {
        std::vector<double> xv, yv;
        for (size_t i = 1; i <= xs.size(); i++) xv.push_back(xs[i]);
        for (size_t i = 1; i <= ys.size(); i++) yv.push_back(ys[i]);
        int count = (int)std::min(xv.size(), yv.size());
        ImPlotSpec spec(ImPlotProp_LineColor, ImVec4(r, g, b, a));
        ImPlot::PlotLine(label_id, xv.data(), yv.data(), count, spec);
    };

    implot["PlotScatter"] = sol::overload(
        [](const char* label_id, sol::table values) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotScatter(label_id, v.data(), (int)v.size());
        },
        [](const char* label_id, sol::table values, double xscale, double xstart) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotScatter(label_id, v.data(), (int)v.size(), xscale, xstart);
        },
        [](const char* label_id, sol::table xs, sol::table ys) {
            std::vector<double> xv, yv;
            for (size_t i = 1; i <= xs.size(); i++) xv.push_back(xs[i]);
            for (size_t i = 1; i <= ys.size(); i++) yv.push_back(ys[i]);
            int count = (int)std::min(xv.size(), yv.size());
            ImPlot::PlotScatter(label_id, xv.data(), yv.data(), count);
        }
    );

    // PlotScatterStyled(label, xs, ys, marker, size, r, g, b, a) - scatter with marker style
    implot["PlotScatterStyled"] = [](const char* label_id, sol::table xs, sol::table ys, int marker, float size, float r, float g, float b, float a) {
        std::vector<double> xv, yv;
        for (size_t i = 1; i <= xs.size(); i++) xv.push_back(xs[i]);
        for (size_t i = 1; i <= ys.size(); i++) yv.push_back(ys[i]);
        int count = (int)std::min(xv.size(), yv.size());
        ImPlotSpec spec(ImPlotProp_Marker, (double)marker, ImPlotProp_MarkerSize, (double)size, ImPlotProp_MarkerFillColor, ImVec4(r, g, b, a));
        ImPlot::PlotScatter(label_id, xv.data(), yv.data(), count, spec);
    };

    implot["PlotStairs"] = sol::overload(
        [](const char* label_id, sol::table values) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotStairs(label_id, v.data(), (int)v.size());
        },
        [](const char* label_id, sol::table xs, sol::table ys) {
            std::vector<double> xv, yv;
            for (size_t i = 1; i <= xs.size(); i++) xv.push_back(xs[i]);
            for (size_t i = 1; i <= ys.size(); i++) yv.push_back(ys[i]);
            int count = (int)std::min(xv.size(), yv.size());
            ImPlot::PlotStairs(label_id, xv.data(), yv.data(), count);
        }
    );

    implot["PlotShaded"] = sol::overload(
        // PlotShaded(label, values) - shade to y=0
        [](const char* label_id, sol::table values) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotShaded(label_id, v.data(), (int)v.size());
        },
        // PlotShaded(label, values, yref)
        [](const char* label_id, sol::table values, double yref) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotShaded(label_id, v.data(), (int)v.size(), yref);
        },
        // PlotShaded(label, xs, ys) - shade to y=0
        [](const char* label_id, sol::table xs, sol::table ys) {
            std::vector<double> xv, yv;
            for (size_t i = 1; i <= xs.size(); i++) xv.push_back(xs[i]);
            for (size_t i = 1; i <= ys.size(); i++) yv.push_back(ys[i]);
            int count = (int)std::min(xv.size(), yv.size());
            ImPlot::PlotShaded(label_id, xv.data(), yv.data(), count);
        }
    );

    // Buffer-based plot functions (zero-copy, uses offset for circular buffer support)
    implot["PlotLineBuffer"] = sol::overload(
        [](const char* label_id, LuaScrollingBuffer& buf) {
            if (buf.Size() > 0) {
                ImPlotSpec spec;
                spec.Offset = buf.Offset;
                ImPlot::PlotLine(label_id, buf.Xs.data(), buf.Ys.data(), buf.Size(), spec);
            }
        },
        [](const char* label_id, LuaRollingBuffer& buf) {
            if (buf.Size() > 0)
                ImPlot::PlotLine(label_id, buf.Xs.data(), buf.Ys.data(), buf.Size());
        }
    );

    implot["PlotShadedBuffer"] = sol::overload(
        [](const char* label_id, LuaScrollingBuffer& buf) {
            if (buf.Size() > 0) {
                ImPlotSpec spec;
                spec.Offset = buf.Offset;
                ImPlot::PlotShaded(label_id, buf.Xs.data(), buf.Ys.data(), buf.Size(), -INFINITY, spec);
            }
        },
        [](const char* label_id, LuaScrollingBuffer& buf, double yref) {
            if (buf.Size() > 0) {
                ImPlotSpec spec;
                spec.Offset = buf.Offset;
                ImPlot::PlotShaded(label_id, buf.Xs.data(), buf.Ys.data(), buf.Size(), yref, spec);
            }
        },
        [](const char* label_id, LuaRollingBuffer& buf) {
            if (buf.Size() > 0)
                ImPlot::PlotShaded(label_id, buf.Xs.data(), buf.Ys.data(), buf.Size());
        }
    );

    implot["PlotScatterBuffer"] = sol::overload(
        [](const char* label_id, LuaScrollingBuffer& buf) {
            if (buf.Size() > 0) {
                ImPlotSpec spec;
                spec.Offset = buf.Offset;
                ImPlot::PlotScatter(label_id, buf.Xs.data(), buf.Ys.data(), buf.Size(), spec);
            }
        },
        [](const char* label_id, LuaRollingBuffer& buf) {
            if (buf.Size() > 0)
                ImPlot::PlotScatter(label_id, buf.Xs.data(), buf.Ys.data(), buf.Size());
        }
    );

    implot["PlotBars"] = sol::overload(
        [](const char* label_id, sol::table values) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotBars(label_id, v.data(), (int)v.size());
        },
        [](const char* label_id, sol::table values, double bar_size) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotBars(label_id, v.data(), (int)v.size(), bar_size);
        },
        [](const char* label_id, sol::table xs, sol::table ys, double bar_size) {
            std::vector<double> xv, yv;
            for (size_t i = 1; i <= xs.size(); i++) xv.push_back(xs[i]);
            for (size_t i = 1; i <= ys.size(); i++) yv.push_back(ys[i]);
            int count = (int)std::min(xv.size(), yv.size());
            ImPlot::PlotBars(label_id, xv.data(), yv.data(), count, bar_size);
        }
    );

    implot["PlotErrorBars"] = sol::overload(
        // symmetric error bars
        [](const char* label_id, sol::table xs, sol::table ys, sol::table err) {
            std::vector<double> xv, yv, ev;
            for (size_t i = 1; i <= xs.size(); i++) xv.push_back(xs[i]);
            for (size_t i = 1; i <= ys.size(); i++) yv.push_back(ys[i]);
            for (size_t i = 1; i <= err.size(); i++) ev.push_back(err[i]);
            int count = (int)std::min({xv.size(), yv.size(), ev.size()});
            ImPlot::PlotErrorBars(label_id, xv.data(), yv.data(), ev.data(), count);
        },
        // asymmetric error bars
        [](const char* label_id, sol::table xs, sol::table ys, sol::table neg, sol::table pos) {
            std::vector<double> xv, yv, nv, pv;
            for (size_t i = 1; i <= xs.size(); i++) xv.push_back(xs[i]);
            for (size_t i = 1; i <= ys.size(); i++) yv.push_back(ys[i]);
            for (size_t i = 1; i <= neg.size(); i++) nv.push_back(neg[i]);
            for (size_t i = 1; i <= pos.size(); i++) pv.push_back(pos[i]);
            int count = (int)std::min({xv.size(), yv.size(), nv.size(), pv.size()});
            ImPlot::PlotErrorBars(label_id, xv.data(), yv.data(), nv.data(), pv.data(), count);
        }
    );

    implot["PlotStems"] = sol::overload(
        [](const char* label_id, sol::table values) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotStems(label_id, v.data(), (int)v.size());
        },
        [](const char* label_id, sol::table xs, sol::table ys) {
            std::vector<double> xv, yv;
            for (size_t i = 1; i <= xs.size(); i++) xv.push_back(xs[i]);
            for (size_t i = 1; i <= ys.size(); i++) yv.push_back(ys[i]);
            int count = (int)std::min(xv.size(), yv.size());
            ImPlot::PlotStems(label_id, xv.data(), yv.data(), count);
        }
    );

    implot["PlotInfLines"] = [](const char* label_id, sol::table values) {
        std::vector<double> v;
        for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
        ImPlot::PlotInfLines(label_id, v.data(), (int)v.size());
    };

    implot["PlotPieChart"] = sol::overload(
        [](sol::table label_ids, sol::table values, double x, double y, double radius) {
            std::vector<const char*> labels;
            std::vector<std::string> label_storage;
            std::vector<double> v;
            for (size_t i = 1; i <= label_ids.size(); i++) {
                label_storage.push_back(label_ids.get<std::string>(i));
            }
            for (auto& s : label_storage) labels.push_back(s.c_str());
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            int count = (int)std::min(labels.size(), v.size());
            ImPlot::PlotPieChart(labels.data(), v.data(), count, x, y, radius);
        },
        [](sol::table label_ids, sol::table values, double x, double y, double radius, const char* label_fmt) {
            std::vector<const char*> labels;
            std::vector<std::string> label_storage;
            std::vector<double> v;
            for (size_t i = 1; i <= label_ids.size(); i++) {
                label_storage.push_back(label_ids.get<std::string>(i));
            }
            for (auto& s : label_storage) labels.push_back(s.c_str());
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            int count = (int)std::min(labels.size(), v.size());
            ImPlot::PlotPieChart(labels.data(), v.data(), count, x, y, radius, label_fmt);
        }
    );

    implot["PlotHeatmap"] = sol::overload(
        [](const char* label_id, sol::table values, int rows, int cols) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotHeatmap(label_id, v.data(), rows, cols);
        },
        [](const char* label_id, sol::table values, int rows, int cols, double scale_min, double scale_max) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotHeatmap(label_id, v.data(), rows, cols, scale_min, scale_max);
        },
        [](const char* label_id, sol::table values, int rows, int cols, double scale_min, double scale_max, const char* label_fmt) {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            ImPlot::PlotHeatmap(label_id, v.data(), rows, cols, scale_min, scale_max, label_fmt);
        }
    );

    implot["PlotHistogram"] = sol::overload(
        [](const char* label_id, sol::table values) -> double {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            return ImPlot::PlotHistogram(label_id, v.data(), (int)v.size());
        },
        [](const char* label_id, sol::table values, int bins) -> double {
            std::vector<double> v;
            for (size_t i = 1; i <= values.size(); i++) v.push_back(values[i]);
            return ImPlot::PlotHistogram(label_id, v.data(), (int)v.size(), bins);
        }
    );

    implot["PlotDigital"] = [](const char* label_id, sol::table xs, sol::table ys) {
        std::vector<double> xv, yv;
        for (size_t i = 1; i <= xs.size(); i++) xv.push_back(xs[i]);
        for (size_t i = 1; i <= ys.size(); i++) yv.push_back(ys[i]);
        int count = (int)std::min(xv.size(), yv.size());
        ImPlot::PlotDigital(label_id, xv.data(), yv.data(), count);
    };

    implot["PlotText"] = sol::overload(
        [](const char* text, double x, double y) { ImPlot::PlotText(text, x, y); },
        [](const char* text, double x, double y, float pix_offset_x, float pix_offset_y) {
            ImPlot::PlotText(text, x, y, ImVec2(pix_offset_x, pix_offset_y));
        }
    );

    implot["PlotDummy"] = [](const char* label_id) { ImPlot::PlotDummy(label_id); };

    // Plot Tools
    implot["DragLineX"] = [](int id, double x, float r, float g, float b, float a) -> std::tuple<bool, double> {
        double val = x;
        bool changed = ImPlot::DragLineX(id, &val, ImVec4(r, g, b, a));
        return std::make_tuple(changed, val);
    };
    implot["DragLineY"] = [](int id, double y, float r, float g, float b, float a) -> std::tuple<bool, double> {
        double val = y;
        bool changed = ImPlot::DragLineY(id, &val, ImVec4(r, g, b, a));
        return std::make_tuple(changed, val);
    };
    implot["DragPoint"] = [](int id, double x, double y, float r, float g, float b, float a) -> std::tuple<bool, double, double> {
        double vx = x, vy = y;
        bool changed = ImPlot::DragPoint(id, &vx, &vy, ImVec4(r, g, b, a));
        return std::make_tuple(changed, vx, vy);
    };

    implot["Annotation"] = sol::overload(
        [](double x, double y, float r, float g, float b, float a, float pix_x, float pix_y, bool clamp) {
            ImPlot::Annotation(x, y, ImVec4(r, g, b, a), ImVec2(pix_x, pix_y), clamp);
        },
        [](double x, double y, float r, float g, float b, float a, float pix_x, float pix_y, bool clamp, const char* fmt) {
            ImPlot::Annotation(x, y, ImVec4(r, g, b, a), ImVec2(pix_x, pix_y), clamp, "%s", fmt);
        }
    );

    implot["TagX"] = sol::overload(
        [](double x, float r, float g, float b, float a) { ImPlot::TagX(x, ImVec4(r, g, b, a)); },
        [](double x, float r, float g, float b, float a, const char* fmt) { ImPlot::TagX(x, ImVec4(r, g, b, a), "%s", fmt); }
    );
    implot["TagY"] = sol::overload(
        [](double y, float r, float g, float b, float a) { ImPlot::TagY(y, ImVec4(r, g, b, a)); },
        [](double y, float r, float g, float b, float a, const char* fmt) { ImPlot::TagY(y, ImVec4(r, g, b, a), "%s", fmt); }
    );

    // Plot Utils
    implot["SetAxis"] = [](int axis) { ImPlot::SetAxis(axis); };
    implot["SetAxes"] = [](int x_axis, int y_axis) { ImPlot::SetAxes(x_axis, y_axis); };

    implot["PixelsToPlot"] = [](float x, float y) -> std::tuple<double, double> {
        ImPlotPoint p = ImPlot::PixelsToPlot(x, y);
        return std::make_tuple(p.x, p.y);
    };
    implot["PlotToPixels"] = [](double x, double y) -> std::tuple<float, float> {
        ImVec2 p = ImPlot::PlotToPixels(x, y);
        return std::make_tuple(p.x, p.y);
    };

    implot["GetPlotPos"] = []() -> std::tuple<float, float> {
        ImVec2 p = ImPlot::GetPlotPos();
        return std::make_tuple(p.x, p.y);
    };
    implot["GetPlotSize"] = []() -> std::tuple<float, float> {
        ImVec2 s = ImPlot::GetPlotSize();
        return std::make_tuple(s.x, s.y);
    };
    implot["GetPlotMousePos"] = []() -> std::tuple<double, double> {
        ImPlotPoint p = ImPlot::GetPlotMousePos();
        return std::make_tuple(p.x, p.y);
    };

    implot["IsPlotHovered"] = &ImPlot::IsPlotHovered;
    implot["IsAxisHovered"] = [](int axis) { return ImPlot::IsAxisHovered(axis); };
    implot["IsSubplotsHovered"] = &ImPlot::IsSubplotsHovered;
    implot["IsPlotSelected"] = &ImPlot::IsPlotSelected;
    implot["CancelPlotSelection"] = &ImPlot::CancelPlotSelection;

    implot["HideNextItem"] = sol::overload(
        []() { ImPlot::HideNextItem(); },
        [](bool hidden) { ImPlot::HideNextItem(hidden); },
        [](bool hidden, int cond) { ImPlot::HideNextItem(hidden, cond); }
    );

    // Legend Utils
    implot["BeginLegendPopup"] = sol::overload(
        [](const char* label_id) { return ImPlot::BeginLegendPopup(label_id); },
        [](const char* label_id, int mouse_button) { return ImPlot::BeginLegendPopup(label_id, mouse_button); }
    );
    implot["EndLegendPopup"] = &ImPlot::EndLegendPopup;
    implot["IsLegendEntryHovered"] = [](const char* label_id) { return ImPlot::IsLegendEntryHovered(label_id); };

    // Styling
    implot["StyleColorsAuto"] = []() { ImPlot::StyleColorsAuto(); };
    implot["StyleColorsClassic"] = []() { ImPlot::StyleColorsClassic(); };
    implot["StyleColorsDark"] = []() { ImPlot::StyleColorsDark(); };
    implot["StyleColorsLight"] = []() { ImPlot::StyleColorsLight(); };

    implot["PushStyleColor"] = [](int idx, float r, float g, float b, float a) {
        ImPlot::PushStyleColor(idx, ImVec4(r, g, b, a));
    };
    implot["PopStyleColor"] = sol::overload(
        []() { ImPlot::PopStyleColor(); },
        [](int count) { ImPlot::PopStyleColor(count); }
    );
    implot["PushStyleVarFloat"] = [](int idx, float val) { ImPlot::PushStyleVar(idx, val); };
    implot["PushStyleVarInt"] = [](int idx, int val) { ImPlot::PushStyleVar(idx, val); };
    implot["PushStyleVarVec2"] = [](int idx, float x, float y) { ImPlot::PushStyleVar(idx, ImVec2(x, y)); };
    implot["PopStyleVar"] = sol::overload(
        []() { ImPlot::PopStyleVar(); },
        [](int count) { ImPlot::PopStyleVar(count); }
    );

    implot["GetLastItemColor"] = []() -> std::tuple<float, float, float, float> {
        ImVec4 c = ImPlot::GetLastItemColor();
        return std::make_tuple(c.x, c.y, c.z, c.w);
    };

    // Colormaps
    implot["PushColormap"] = sol::overload(
        [](int cmap) { ImPlot::PushColormap(cmap); },
        [](const char* name) { ImPlot::PushColormap(name); }
    );
    implot["PopColormap"] = sol::overload(
        []() { ImPlot::PopColormap(); },
        [](int count) { ImPlot::PopColormap(count); }
    );
    implot["NextColormapColor"] = []() -> std::tuple<float, float, float, float> {
        ImVec4 c = ImPlot::NextColormapColor();
        return std::make_tuple(c.x, c.y, c.z, c.w);
    };
    implot["GetColormapCount"] = &ImPlot::GetColormapCount;
    implot["GetColormapName"] = [](int cmap) { return ImPlot::GetColormapName(cmap); };
    implot["GetColormapIndex"] = [](const char* name) { return ImPlot::GetColormapIndex(name); };
    implot["GetColormapSize"] = sol::overload(
        []() { return ImPlot::GetColormapSize(); },
        [](int cmap) { return ImPlot::GetColormapSize(cmap); }
    );
    implot["GetColormapColor"] = sol::overload(
        [](int idx) -> std::tuple<float, float, float, float> {
            ImVec4 c = ImPlot::GetColormapColor(idx);
            return std::make_tuple(c.x, c.y, c.z, c.w);
        },
        [](int idx, int cmap) -> std::tuple<float, float, float, float> {
            ImVec4 c = ImPlot::GetColormapColor(idx, cmap);
            return std::make_tuple(c.x, c.y, c.z, c.w);
        }
    );
    implot["SampleColormap"] = sol::overload(
        [](float t) -> std::tuple<float, float, float, float> {
            ImVec4 c = ImPlot::SampleColormap(t);
            return std::make_tuple(c.x, c.y, c.z, c.w);
        },
        [](float t, int cmap) -> std::tuple<float, float, float, float> {
            ImVec4 c = ImPlot::SampleColormap(t, cmap);
            return std::make_tuple(c.x, c.y, c.z, c.w);
        }
    );
    implot["ColormapScale"] = sol::overload(
        [](const char* label, double scale_min, double scale_max) {
            ImPlot::ColormapScale(label, scale_min, scale_max);
        },
        [](const char* label, double scale_min, double scale_max, float w, float h) {
            ImPlot::ColormapScale(label, scale_min, scale_max, ImVec2(w, h));
        }
    );
    implot["ColormapButton"] = sol::overload(
        [](const char* label) { return ImPlot::ColormapButton(label); },
        [](const char* label, float w, float h) { return ImPlot::ColormapButton(label, ImVec2(w, h)); }
    );

    implot["BustColorCache"] = sol::overload(
        []() { ImPlot::BustColorCache(); },
        [](const char* plot_title_id) { ImPlot::BustColorCache(plot_title_id); }
    );

    // Axis enum constants
    implot["Axis_X1"] = ImAxis_X1;
    implot["Axis_X2"] = ImAxis_X2;
    implot["Axis_X3"] = ImAxis_X3;
    implot["Axis_Y1"] = ImAxis_Y1;
    implot["Axis_Y2"] = ImAxis_Y2;
    implot["Axis_Y3"] = ImAxis_Y3;

    // Plot flags
    implot["Flags_None"] = ImPlotFlags_None;
    implot["Flags_NoTitle"] = ImPlotFlags_NoTitle;
    implot["Flags_NoLegend"] = ImPlotFlags_NoLegend;
    implot["Flags_NoMouseText"] = ImPlotFlags_NoMouseText;
    implot["Flags_NoInputs"] = ImPlotFlags_NoInputs;
    implot["Flags_NoMenus"] = ImPlotFlags_NoMenus;
    implot["Flags_NoBoxSelect"] = ImPlotFlags_NoBoxSelect;
    implot["Flags_NoFrame"] = ImPlotFlags_NoFrame;
    implot["Flags_Equal"] = ImPlotFlags_Equal;
    implot["Flags_Crosshairs"] = ImPlotFlags_Crosshairs;
    implot["Flags_CanvasOnly"] = ImPlotFlags_CanvasOnly;

    // Axis flags
    implot["AxisFlags_None"] = ImPlotAxisFlags_None;
    implot["AxisFlags_NoLabel"] = ImPlotAxisFlags_NoLabel;
    implot["AxisFlags_NoGridLines"] = ImPlotAxisFlags_NoGridLines;
    implot["AxisFlags_NoTickMarks"] = ImPlotAxisFlags_NoTickMarks;
    implot["AxisFlags_NoTickLabels"] = ImPlotAxisFlags_NoTickLabels;
    implot["AxisFlags_NoInitialFit"] = ImPlotAxisFlags_NoInitialFit;
    implot["AxisFlags_NoMenus"] = ImPlotAxisFlags_NoMenus;
    implot["AxisFlags_NoSideSwitch"] = ImPlotAxisFlags_NoSideSwitch;
    implot["AxisFlags_NoHighlight"] = ImPlotAxisFlags_NoHighlight;
    implot["AxisFlags_Opposite"] = ImPlotAxisFlags_Opposite;
    implot["AxisFlags_Foreground"] = ImPlotAxisFlags_Foreground;
    implot["AxisFlags_Invert"] = ImPlotAxisFlags_Invert;
    implot["AxisFlags_AutoFit"] = ImPlotAxisFlags_AutoFit;
    implot["AxisFlags_RangeFit"] = ImPlotAxisFlags_RangeFit;
    implot["AxisFlags_PanStretch"] = ImPlotAxisFlags_PanStretch;
    implot["AxisFlags_LockMin"] = ImPlotAxisFlags_LockMin;
    implot["AxisFlags_LockMax"] = ImPlotAxisFlags_LockMax;
    implot["AxisFlags_Lock"] = ImPlotAxisFlags_Lock;
    implot["AxisFlags_NoDecorations"] = ImPlotAxisFlags_NoDecorations;
    implot["AxisFlags_AuxDefault"] = ImPlotAxisFlags_AuxDefault;

    // Legend flags
    implot["LegendFlags_None"] = ImPlotLegendFlags_None;
    implot["LegendFlags_NoButtons"] = ImPlotLegendFlags_NoButtons;
    implot["LegendFlags_NoHighlightItem"] = ImPlotLegendFlags_NoHighlightItem;
    implot["LegendFlags_NoHighlightAxis"] = ImPlotLegendFlags_NoHighlightAxis;
    implot["LegendFlags_NoMenus"] = ImPlotLegendFlags_NoMenus;
    implot["LegendFlags_Outside"] = ImPlotLegendFlags_Outside;
    implot["LegendFlags_Horizontal"] = ImPlotLegendFlags_Horizontal;
    implot["LegendFlags_Sort"] = ImPlotLegendFlags_Sort;

    // Line flags
    implot["LineFlags_None"] = ImPlotLineFlags_None;
    implot["LineFlags_Segments"] = ImPlotLineFlags_Segments;
    implot["LineFlags_Loop"] = ImPlotLineFlags_Loop;
    implot["LineFlags_SkipNaN"] = ImPlotLineFlags_SkipNaN;
    implot["LineFlags_NoClip"] = ImPlotLineFlags_NoClip;
    implot["LineFlags_Shaded"] = ImPlotLineFlags_Shaded;

    // Bars flags
    implot["BarsFlags_None"] = ImPlotBarsFlags_None;
    implot["BarsFlags_Horizontal"] = ImPlotBarsFlags_Horizontal;

    // Location
    implot["Location_Center"] = ImPlotLocation_Center;
    implot["Location_North"] = ImPlotLocation_North;
    implot["Location_South"] = ImPlotLocation_South;
    implot["Location_West"] = ImPlotLocation_West;
    implot["Location_East"] = ImPlotLocation_East;
    implot["Location_NorthWest"] = ImPlotLocation_NorthWest;
    implot["Location_NorthEast"] = ImPlotLocation_NorthEast;
    implot["Location_SouthWest"] = ImPlotLocation_SouthWest;
    implot["Location_SouthEast"] = ImPlotLocation_SouthEast;

    // Condition
    implot["Cond_None"] = ImPlotCond_None;
    implot["Cond_Always"] = ImPlotCond_Always;
    implot["Cond_Once"] = ImPlotCond_Once;

    // Marker
    implot["Marker_None"] = ImPlotMarker_None;
    implot["Marker_Auto"] = ImPlotMarker_Auto;
    implot["Marker_Circle"] = ImPlotMarker_Circle;
    implot["Marker_Square"] = ImPlotMarker_Square;
    implot["Marker_Diamond"] = ImPlotMarker_Diamond;
    implot["Marker_Up"] = ImPlotMarker_Up;
    implot["Marker_Down"] = ImPlotMarker_Down;
    implot["Marker_Left"] = ImPlotMarker_Left;
    implot["Marker_Right"] = ImPlotMarker_Right;
    implot["Marker_Cross"] = ImPlotMarker_Cross;
    implot["Marker_Plus"] = ImPlotMarker_Plus;
    implot["Marker_Asterisk"] = ImPlotMarker_Asterisk;

    // Scale
    implot["Scale_Linear"] = ImPlotScale_Linear;
    implot["Scale_Time"] = ImPlotScale_Time;
    implot["Scale_Log10"] = ImPlotScale_Log10;
    implot["Scale_SymLog"] = ImPlotScale_SymLog;

    // Colormap
    implot["Colormap_Deep"] = ImPlotColormap_Deep;
    implot["Colormap_Dark"] = ImPlotColormap_Dark;
    implot["Colormap_Pastel"] = ImPlotColormap_Pastel;
    implot["Colormap_Paired"] = ImPlotColormap_Paired;
    implot["Colormap_Viridis"] = ImPlotColormap_Viridis;
    implot["Colormap_Plasma"] = ImPlotColormap_Plasma;
    implot["Colormap_Hot"] = ImPlotColormap_Hot;
    implot["Colormap_Cool"] = ImPlotColormap_Cool;
    implot["Colormap_Pink"] = ImPlotColormap_Pink;
    implot["Colormap_Jet"] = ImPlotColormap_Jet;
    implot["Colormap_Twilight"] = ImPlotColormap_Twilight;
    implot["Colormap_RdBu"] = ImPlotColormap_RdBu;
    implot["Colormap_BrBG"] = ImPlotColormap_BrBG;
    implot["Colormap_PiYG"] = ImPlotColormap_PiYG;
    implot["Colormap_Spectral"] = ImPlotColormap_Spectral;
    implot["Colormap_Greys"] = ImPlotColormap_Greys;

    // Col (style colors)
    implot["Col_FrameBg"] = ImPlotCol_FrameBg;
    implot["Col_PlotBg"] = ImPlotCol_PlotBg;
    implot["Col_PlotBorder"] = ImPlotCol_PlotBorder;
    implot["Col_LegendBg"] = ImPlotCol_LegendBg;
    implot["Col_LegendBorder"] = ImPlotCol_LegendBorder;
    implot["Col_LegendText"] = ImPlotCol_LegendText;
    implot["Col_TitleText"] = ImPlotCol_TitleText;
    implot["Col_InlayText"] = ImPlotCol_InlayText;
    implot["Col_AxisText"] = ImPlotCol_AxisText;
    implot["Col_AxisGrid"] = ImPlotCol_AxisGrid;
    implot["Col_AxisTick"] = ImPlotCol_AxisTick;
    implot["Col_AxisBg"] = ImPlotCol_AxisBg;
    implot["Col_AxisBgHovered"] = ImPlotCol_AxisBgHovered;
    implot["Col_AxisBgActive"] = ImPlotCol_AxisBgActive;
    implot["Col_Selection"] = ImPlotCol_Selection;
    implot["Col_Crosshairs"] = ImPlotCol_Crosshairs;

    // Histogram bin methods
    implot["Bin_Sqrt"] = ImPlotBin_Sqrt;
    implot["Bin_Sturges"] = ImPlotBin_Sturges;
    implot["Bin_Rice"] = ImPlotBin_Rice;
    implot["Bin_Scott"] = ImPlotBin_Scott;
}
