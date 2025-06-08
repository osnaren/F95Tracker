#include "window.h"
#include "util.h"

#include <app.h>

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

    ImGui_Text("FPS: %.1f", gui->io->Framerate);

    ImGui_Spacing();

    ImGui_Text("DB test/demo:");

    ImGui_AlignTextToFramePadding();
    ImGui_Text("Browser Custom Arguments:");
    ImGui_SameLine();
    ImGui_SetNextItemWidth(690.0f);
    ImGui_InputTextMString(
        "###browser_custom_arguments",
        app.settings->browser_custom_arguments,
        ImGuiInputTextFlags_None);
    if(ImGui_IsItemDeactivatedAfterEdit()) {
        db_save_setting(app.db, app.settings, SettingsColumn_browser_custom_arguments);
    }

    ImGui_AlignTextToFramePadding();
    ImGui_Text("BG interval:");
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
        db_save_setting(app.db, app.settings, SettingsColumn_bg_refresh_interval);
    }

    ImGui_AlignTextToFramePadding();
    ImGui_Text("Accent color:");
    ImGui_SameLine();
    ImGui_SetNextItemWidth(200.0f);
    ImGui_ColorEdit3(
        "###style_accent",
        (flt32_t*)&app.settings->style_accent.Value,
        ImGuiColorEditFlags_None);
    if(ImGui_IsItemDeactivatedAfterEdit()) {
        db_save_setting(app.db, app.settings, SettingsColumn_style_accent);
    }

    if(ImGui_Button("Test save JSON fields")) {
        db_save_setting(app.db, app.settings, SettingsColumn_default_exe_dir);
        db_save_setting(app.db, app.settings, SettingsColumn_downloads_dir);
        db_save_setting(app.db, app.settings, SettingsColumn_tags_highlights);
        db_save_setting(app.db, app.settings, SettingsColumn_manual_sort_list);
        db_save_setting(app.db, app.settings, SettingsColumn_hidden_timeline_events);
    }

    ImGui_BeginGroup();
    ImGui_AlignTextToFramePadding();
    ImGui_Text("Tabs:");
    if(ImGui_Button("Test add new tab")) {
        Tab_ptr tab = db_create_tab(app.db, app.tabs);
        UNUSED(tab);
    }
    for each(Tab_ptr, tab, TabList, app.tabs) {
        ImGui_PushIDInt(tab->id);
        if(ImGui_Button(mdi_trash_can_outline)) {
            db_delete_tab(app.db, tab, app.tabs);
            ImGui_PopID();
            continue;
        }
        ImGui_SameLine();
        ImGui_Text("%d %d %s", tab->position, tab->id, m_string_get_cstr(tab->name));
        ImGui_PopID();
    }
    ImGui_EndGroup();

    ImGui_SameLine();

    ImGui_BeginGroup();
    ImGui_AlignTextToFramePadding();
    ImGui_Text("Labels:");
    if(ImGui_Button("Test add new label")) {
        Label_ptr label = db_create_label(app.db, app.labels);
        UNUSED(label);
    }
    for each(Label_ptr, label, LabelList, app.labels) {
        ImGui_PushIDInt(label->id);
        if(ImGui_Button(mdi_trash_can_outline)) {
            db_delete_label(app.db, label, app.labels);
            ImGui_PopID();
            continue;
        }
        ImGui_SameLine();
        ImGui_Text("%d %d %s", label->position, label->id, m_string_get_cstr(label->name));
        ImGui_PopID();
    }
    ImGui_EndGroup();

    for each(GameDict_pair, pair, GameDict, app.games) {
        Game* game = pair.value;
        ImGui_Text("%d %s", game->id, m_string_get_cstr(game->name));
        for each(Label_ptr, label, LabelPtrList, game->labels) {
            ImGui_SameLine();
            ImGui_TextUnformatted(m_string_get_cstr(label->name));
        }
    }

    ImGui_End();
}
