#include "lua_imgui_bindings.h"
#include <string>
#include <tuple>

void lua_bsge_init_imgui_bindings(sol::state &lua) {
    // Create ImGui namespace
    auto imgui_namespace = lua["ImGui"].get_or_create<sol::table>();
    
    // Window functions
    imgui_namespace["Begin"] = sol::overload(
        [](const char* name) { return ImGui::Begin(name); },
        [](const char* name, bool* p_open) { return ImGui::Begin(name, p_open); },
        [](const char* name, bool* p_open, int flags) { return ImGui::Begin(name, p_open, flags); }
    );
    imgui_namespace["End"] = &ImGui::End;

    // TODO: expose ImGuiCol_Text and shit and write a universal function
    imgui_namespace["PushTextColor"] =  [](glm::vec4 color) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r, color.g, color.b, color.w)); };
    imgui_namespace["PopStyleColor"] =  []() { ImGui::PopStyleColor(1); };
    
    // Basic widgets
    imgui_namespace["Text"] = [](const char* text) { ImGui::Text("%s", text); };
    imgui_namespace["TextColored"] = [](float r, float g, float b, float a, const char* text) {
        ImGui::TextColored(ImVec4(r, g, b, a), "%s", text);
    };
    imgui_namespace["TextDisabled"] = [](const char* text) { ImGui::TextDisabled("%s", text); };
    imgui_namespace["TextWrapped"] = [](const char* text) { ImGui::TextWrapped("%s", text); };
    imgui_namespace["LabelText"] = [](const char* label, const char* text) { ImGui::LabelText(label, "%s", text); };
    imgui_namespace["BulletText"] = [](const char* text) { ImGui::BulletText("%s", text); };
    
    // Buttons
    imgui_namespace["Button"] = sol::overload(
        [](const char* label) { return ImGui::Button(label); },
        [](const char* label, float size_x, float size_y) { return ImGui::Button(label, ImVec2(size_x, size_y)); }
    );
    imgui_namespace["SmallButton"] = &ImGui::SmallButton;
    imgui_namespace["InvisibleButton"] = [](const char* id, float size_x, float size_y) {
        return ImGui::InvisibleButton(id, ImVec2(size_x, size_y));
    };
    imgui_namespace["ArrowButton"] = &ImGui::ArrowButton;
    imgui_namespace["Checkbox"] = [](const char* label, bool value) -> std::tuple<bool, bool> {
        bool v = value;
        bool result = ImGui::Checkbox(label, &v);
        return std::make_tuple(result, v);
    };
    
    // Input widgets  
    imgui_namespace["InputText"] = [](const char* label, const std::string& text) -> std::tuple<bool, std::string> {
        char buffer[1024];
        strncpy(buffer, text.c_str(), sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        bool result = ImGui::InputText(label, buffer, sizeof(buffer));
        return std::make_tuple(result, std::string(buffer));
    };
    
    imgui_namespace["InputFloat"] = [](const char* label, float value) -> std::tuple<bool, float> {
        float v = value;
        bool result = ImGui::InputFloat(label, &v);
        return std::make_tuple(result, v);
    };
    imgui_namespace["InputInt"] = [](const char* label, int value) -> std::tuple<bool, int> {
        int v = value;
        bool result = ImGui::InputInt(label, &v);
        return std::make_tuple(result, v);
    };
    
    // Sliders
    imgui_namespace["SliderFloat"] = [](const char* label, float value, float v_min, float v_max) -> std::tuple<bool, float> {
        float v = value;
        bool result = ImGui::SliderFloat(label, &v, v_min, v_max);
        return std::make_tuple(result, v);
    };
    imgui_namespace["SliderInt"] = [](const char* label, int value, int v_min, int v_max) -> std::tuple<bool, int> {
        int v = value;
        bool result = ImGui::SliderInt(label, &v, v_min, v_max);
        return std::make_tuple(result, v);
    };
    
    // Color widgets
    imgui_namespace["ColorEdit3"] = [](const char* label, float r, float g, float b) -> std::tuple<bool, float, float, float> {
        float col[3] = {r, g, b};
        bool result = ImGui::ColorEdit3(label, col);
        return std::make_tuple(result, col[0], col[1], col[2]);
    };
    imgui_namespace["ColorEdit4"] = [](const char* label, float r, float g, float b, float a) -> std::tuple<bool, float, float, float, float> {
        float col[4] = {r, g, b, a};
        bool result = ImGui::ColorEdit4(label, col);
        return std::make_tuple(result, col[0], col[1], col[2], col[3]);
    };
    
    // Trees and selectables
    imgui_namespace["TreeNode"] = sol::overload(
        [](const char* label) { return ImGui::TreeNode(label); },
        [](const char* str_id, const char* fmt) { return ImGui::TreeNode(str_id, "%s", fmt); }
    );
    imgui_namespace["TreeNodeEx"] = [](const char* label, int flags) {
        return ImGui::TreeNodeEx(label, flags);
    };
    imgui_namespace["TreePop"] = &ImGui::TreePop;
    imgui_namespace["TreePush"] = [](const char* str_id) { ImGui::TreePush(str_id); };
    imgui_namespace["SetNextItemOpen"] = sol::overload(
        [](bool is_open) { ImGui::SetNextItemOpen(is_open); },
        [](bool is_open, int cond) { ImGui::SetNextItemOpen(is_open, cond); }
    );
    
    imgui_namespace["Selectable"] = sol::overload(
        [](const char* label) { return ImGui::Selectable(label); },
        [](const char* label, bool selected) -> std::tuple<bool, bool> {
            bool sel = selected;
            bool result = ImGui::Selectable(label, &sel);
            return std::make_tuple(result, sel);
        }
    );
    
    // Layout
    imgui_namespace["Separator"] = &ImGui::Separator;
    imgui_namespace["SameLine"] = sol::overload(
        []() { ImGui::SameLine(); },
        [](float offset_from_start_x) { ImGui::SameLine(offset_from_start_x); },
        [](float offset_from_start_x, float spacing) { ImGui::SameLine(offset_from_start_x, spacing); }
    );
    imgui_namespace["NewLine"] = &ImGui::NewLine;
    imgui_namespace["Spacing"] = &ImGui::Spacing;
    imgui_namespace["Image"] = [](int user_texture, glm::vec2 pos, bool invert = false) {
        ImGui::Image((void *)user_texture, ImVec2(pos.x, pos.y), invert ? ImVec2(0, 0) : ImVec2(0, 1), invert ? ImVec2(1, 1) : ImVec2(1, 0));
    };

    imgui_namespace["Dummy"] = [](float size_x, float size_y) { ImGui::Dummy(ImVec2(size_x, size_y)); };
    imgui_namespace["Indent"] = sol::overload(
        []() { ImGui::Indent(); },
        [](float indent_w) { ImGui::Indent(indent_w); }
    );
    imgui_namespace["Unindent"] = sol::overload(
        []() { ImGui::Unindent(); },
        [](float indent_w) { ImGui::Unindent(indent_w); }
    );
    
    // Groups
    imgui_namespace["BeginGroup"] = &ImGui::BeginGroup;
    imgui_namespace["EndGroup"] = &ImGui::EndGroup;
    
    // Cursor/Layout
    imgui_namespace["GetCursorPos"] = []() { 
        ImVec2 pos = ImGui::GetCursorPos();
        return std::make_tuple(pos.x, pos.y);
    };
    imgui_namespace["SetCursorPos"] = [](float x, float y) { ImGui::SetCursorPos(ImVec2(x, y)); };
    imgui_namespace["GetCursorPosX"] = &ImGui::GetCursorPosX;
    imgui_namespace["GetCursorPosY"] = &ImGui::GetCursorPosY;
    imgui_namespace["SetCursorPosX"] = &ImGui::SetCursorPosX;
    imgui_namespace["SetCursorPosY"] = &ImGui::SetCursorPosY;
    
    // Child windows
    imgui_namespace["BeginChild"] = sol::overload(
        [](const char* str_id) { return ImGui::BeginChild(str_id); },
        [](const char* str_id, float size_x, float size_y) { return ImGui::BeginChild(str_id, ImVec2(size_x, size_y)); },
        [](const char* str_id, float size_x, float size_y, bool border) { return ImGui::BeginChild(str_id, ImVec2(size_x, size_y), border); }
    );
    imgui_namespace["EndChild"] = &ImGui::EndChild;
    
    // Scrolling
    imgui_namespace["GetScrollX"] = &ImGui::GetScrollX;
    imgui_namespace["GetScrollY"] = &ImGui::GetScrollY;
    imgui_namespace["SetScrollX"] = &ImGui::SetScrollX;
    imgui_namespace["SetScrollY"] = &ImGui::SetScrollY;
    imgui_namespace["GetScrollMaxX"] = &ImGui::GetScrollMaxX;
    imgui_namespace["GetScrollMaxY"] = &ImGui::GetScrollMaxY;
    imgui_namespace["SetScrollHereX"] = sol::overload(
        []() { ImGui::SetScrollHereX(); },
        [](float center_x_ratio) { ImGui::SetScrollHereX(center_x_ratio); }
    );
    imgui_namespace["SetScrollHereY"] = sol::overload(
        []() { ImGui::SetScrollHereY(); },
        [](float center_y_ratio) { ImGui::SetScrollHereY(center_y_ratio); }
    );
    
    // Utilities
    imgui_namespace["IsItemHovered"] = &ImGui::IsItemHovered;
    imgui_namespace["IsItemActive"] = &ImGui::IsItemActive;
    imgui_namespace["IsItemFocused"] = &ImGui::IsItemFocused;
    imgui_namespace["IsItemClicked"] = sol::overload(
        []() { return ImGui::IsItemClicked(); },
        [](int mouse_button) { return ImGui::IsItemClicked(mouse_button); }
    );
    imgui_namespace["IsItemVisible"] = &ImGui::IsItemVisible;
    imgui_namespace["IsItemEdited"] = &ImGui::IsItemEdited;
    imgui_namespace["IsItemActivated"] = &ImGui::IsItemActivated;
    imgui_namespace["IsItemDeactivated"] = &ImGui::IsItemDeactivated;
    imgui_namespace["IsItemDeactivatedAfterEdit"] = &ImGui::IsItemDeactivatedAfterEdit;
    imgui_namespace["IsItemToggledOpen"] = &ImGui::IsItemToggledOpen;
    imgui_namespace["IsAnyItemHovered"] = &ImGui::IsAnyItemHovered;
    imgui_namespace["IsAnyItemActive"] = &ImGui::IsAnyItemActive;
    imgui_namespace["IsAnyItemFocused"] = &ImGui::IsAnyItemFocused;
    
    // Window utilities
    imgui_namespace["IsWindowAppearing"] = &ImGui::IsWindowAppearing;
    imgui_namespace["IsWindowCollapsed"] = &ImGui::IsWindowCollapsed;
    imgui_namespace["IsWindowFocused"] = sol::overload(
        []() { return ImGui::IsWindowFocused(); },
        [](int flags) { return ImGui::IsWindowFocused(flags); }
    );
    imgui_namespace["IsWindowHovered"] = sol::overload(
        []() { return ImGui::IsWindowHovered(); },
        [](int flags) { return ImGui::IsWindowHovered(flags); }
    );
    
    imgui_namespace["GetWindowPos"] = []() { 
        ImVec2 pos = ImGui::GetWindowPos();
        return std::make_tuple(pos.x, pos.y);
    };
    imgui_namespace["GetWindowSize"] = []() { 
        ImVec2 size = ImGui::GetWindowSize();
        return std::make_tuple(size.x, size.y);
    };
    imgui_namespace["GetWindowWidth"] = &ImGui::GetWindowWidth;
    imgui_namespace["GetWindowHeight"] = &ImGui::GetWindowHeight;
    
    // Commonly used flags as constants
    imgui_namespace["WindowFlags_None"] = ImGuiWindowFlags_None;
    imgui_namespace["WindowFlags_NoTitleBar"] = ImGuiWindowFlags_NoTitleBar;
    imgui_namespace["WindowFlags_NoResize"] = ImGuiWindowFlags_NoResize;
    imgui_namespace["WindowFlags_NoMove"] = ImGuiWindowFlags_NoMove;
    imgui_namespace["WindowFlags_NoScrollbar"] = ImGuiWindowFlags_NoScrollbar;
    imgui_namespace["WindowFlags_NoScrollWithMouse"] = ImGuiWindowFlags_NoScrollWithMouse;
    imgui_namespace["WindowFlags_NoCollapse"] = ImGuiWindowFlags_NoCollapse;
    imgui_namespace["WindowFlags_AlwaysAutoResize"] = ImGuiWindowFlags_AlwaysAutoResize;
    imgui_namespace["WindowFlags_NoBackground"] = ImGuiWindowFlags_NoBackground;
    imgui_namespace["WindowFlags_NoSavedSettings"] = ImGuiWindowFlags_NoSavedSettings;
    imgui_namespace["WindowFlags_NoMouseInputs"] = ImGuiWindowFlags_NoMouseInputs;
    imgui_namespace["WindowFlags_MenuBar"] = ImGuiWindowFlags_MenuBar;
    imgui_namespace["WindowFlags_HorizontalScrollbar"] = ImGuiWindowFlags_HorizontalScrollbar;
    imgui_namespace["WindowFlags_NoFocusOnAppearing"] = ImGuiWindowFlags_NoFocusOnAppearing;
    imgui_namespace["WindowFlags_NoBringToFrontOnFocus"] = ImGuiWindowFlags_NoBringToFrontOnFocus;
    imgui_namespace["WindowFlags_AlwaysVerticalScrollbar"] = ImGuiWindowFlags_AlwaysVerticalScrollbar;
    imgui_namespace["WindowFlags_AlwaysHorizontalScrollbar"] = ImGuiWindowFlags_AlwaysHorizontalScrollbar;
    imgui_namespace["WindowFlags_AlwaysUseWindowPadding"] = ImGuiWindowFlags_AlwaysUseWindowPadding;
    
    // Tree flags
    imgui_namespace["TreeNodeFlags_None"] = ImGuiTreeNodeFlags_None;
    imgui_namespace["TreeNodeFlags_Selected"] = ImGuiTreeNodeFlags_Selected;
    imgui_namespace["TreeNodeFlags_Framed"] = ImGuiTreeNodeFlags_Framed;
    imgui_namespace["TreeNodeFlags_AllowItemOverlap"] = ImGuiTreeNodeFlags_AllowItemOverlap;
    imgui_namespace["TreeNodeFlags_NoTreePushOnOpen"] = ImGuiTreeNodeFlags_NoTreePushOnOpen;
    imgui_namespace["TreeNodeFlags_NoAutoOpenOnLog"] = ImGuiTreeNodeFlags_NoAutoOpenOnLog;
    imgui_namespace["TreeNodeFlags_DefaultOpen"] = ImGuiTreeNodeFlags_DefaultOpen;
    imgui_namespace["TreeNodeFlags_OpenOnDoubleClick"] = ImGuiTreeNodeFlags_OpenOnDoubleClick;
    imgui_namespace["TreeNodeFlags_OpenOnArrow"] = ImGuiTreeNodeFlags_OpenOnArrow;
    imgui_namespace["TreeNodeFlags_Leaf"] = ImGuiTreeNodeFlags_Leaf;
    imgui_namespace["TreeNodeFlags_Bullet"] = ImGuiTreeNodeFlags_Bullet;
    imgui_namespace["TreeNodeFlags_FramePadding"] = ImGuiTreeNodeFlags_FramePadding;
    imgui_namespace["TreeNodeFlags_SpanAvailWidth"] = ImGuiTreeNodeFlags_SpanAvailWidth;
    imgui_namespace["TreeNodeFlags_SpanFullWidth"] = ImGuiTreeNodeFlags_SpanFullWidth;
    imgui_namespace["TreeNodeFlags_NavLeftJumpsBackHere"] = ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
}