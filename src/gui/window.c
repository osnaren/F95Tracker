#include "window.h"

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

    ImGui_End();
}
