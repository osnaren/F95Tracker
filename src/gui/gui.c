#include "gui.h"
#include "backend.h"
#include "fonts.h"
#include "window.h"

Gui* gui_init(void) {
    Gui* gui = malloc(sizeof(Gui));
    gui->should_close = false;

    if(!gui_backend_init(gui, "F95Checker WIP C Rewrite", 1280, 720)) {
        free(gui);
        return NULL;
    }

    m_string_init(gui->str_test);

    gui->io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    gui->io->IniFilename = NULL;
    gui->io->LogFilename = NULL;
    gui->io->ConfigDragClickToInputText = true;
    ImGui_StyleColorsDark(NULL);
    gui->style->ScrollbarSize = 10.0f;
    gui->style->FrameBorderSize = 1.0f;
    gui->style->ItemSpacing.x = gui->style->ItemSpacing.y;
    gui->style->Colors[ImGuiCol_ModalWindowDimBg] = (ImVec4){0.0f, 0.0f, 0.0f, 0.5f};

    gui_fonts_load(gui);

    return gui;
}

bool gui_should_close(Gui* gui) {
    return gui->should_close;
}

void gui_tick(Gui* gui) {
    gui_backend_process_events(gui);
    if(gui->should_close) {
        return;
    }
    gui_backend_new_frame(gui);
    gui_window_draw(gui);
    gui_backend_render(gui);
}

void gui_free(Gui* gui) {
    m_string_clear(gui->str_test);
    gui_backend_shutdown(gui);
    free(gui);
}
