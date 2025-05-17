#include "window.h"
#include "app.h"
#include "util.h"

const char* ok = mdi_check " Ok";
const char* cancel = mdi_cancel " Cancel";

void gui_window_draw(Gui* gui) {
    ImGui_SetNextWindowPos((ImVec2){0.0f, 0.0f}, ImGuiCond_Once);
    int32_t width, height;
    SDL_GetWindowSize(gui->window, &width, &height);
    const ImVec2 window_size = {width, height};
    if(window_size.x != gui->prev_size.x || window_size.y != gui->prev_size.y) {
        ImGui_SetNextWindowSize(window_size, ImGuiCond_Always);
    }

    ImGui_PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui_Begin(
        "F95Checker",
        NULL,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse);
    ImGui_PopStyleVar();

    ImGui_Text("Hello, World!");

    ImGui_Text("Version: %s WIP", app.version);

    ImGui_AlignTextToFramePadding();
    ImGui_Text("Browser Custom Arguments (DB test/demo):");
    ImGui_SameLine();
    ImGui_SetNextItemWidth(690.0f);
    ImGui_InputTextMstring(
        "###browser_custom_arguments",
        app.settings->browser_custom_arguments,
        ImGuiInputTextFlags_None);
    if(ImGui_IsItemDeactivatedAfterEdit()) {
        db_save_settings(app.db, app.settings, SettingsColumn_browser_custom_arguments);
    }

    ImGui_AlignTextToFramePadding();
    ImGui_Text("BG interval (DB test/demo):");
    ImGui_SameLine();
    ImGui_SetNextItemWidth(100.0f);
    ImGui_DragIntEx(
        "###bg_refresh_interval",
        &app.settings->bg_refresh_interval,
        4.0,
        30,
        1440,
        "%d min",
        ImGuiSliderFlags_None);
    if(ImGui_IsItemDeactivatedAfterEdit()) {
        db_save_settings(app.db, app.settings, SettingsColumn_bg_refresh_interval);
    }

    ImGui_End();
}
