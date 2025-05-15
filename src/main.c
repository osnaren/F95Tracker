#include "gui/gui.h"

int32_t main(int32_t argc, char** argv) {
    UNUSED(argc);
    UNUSED(argv);
    int32_t ret = 0;

    Gui* gui = gui_init();
    if(gui == NULL) {
        ret = 1;
        goto exit_gui;
    }

    while(!gui_should_close(gui)) {
        gui_tick(gui);
    }

    gui_free(gui);
exit_gui:

    return ret;
}
