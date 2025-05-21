#include <app.h>

App app = {
    .version = F95CHECKER_VERSION,
#if OS == OS_WINDOWS
    .os = Os_Windows,
#elif OS == OS_LINUX
    .os = Os_Linux,
#elif OS == OS_MACOS
    .os = Os_MacOS,
#endif
};

int32_t main(int32_t argc, char** argv) {
    UNUSED(argc);
    UNUSED(argv);
    int32_t ret = 0;

    app.db = db_init();
    if(app.db == NULL) {
        ret = 1;
        goto exit_db;
    }

    TabList_init(app.tabs);
    db_load_tabs(app.db, &app.tabs);

    app.settings = settings_init();
    db_load_settings(app.db, app.settings);

    app.gui = gui_init();
    if(app.gui == NULL) {
        ret = 1;
        goto exit_gui;
    }

    while(!gui_should_close(app.gui)) {
        gui_tick(app.gui);
    }

    gui_free(app.gui);
exit_gui:

    settings_free(app.settings);

    TabList_clear(app.tabs);

    db_free(app.db);
exit_db:

    return ret;
}
