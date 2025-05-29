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

    LabelList_init(app.labels);
    db_load_labels(app.db, &app.labels);

    app.settings = settings_init();
    db_load_settings(app.db, app.settings);

    GameDict_init(app.games);
    db_load_games(app.db, &app.games);

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

    db_free(app.db);

    for
        M_EACH(pair, app.games, GameDict_t) {
            game_free(pair->value);
        }
    GameDict_clear(app.games);

    settings_free(app.settings);

    LabelList_clear(app.labels);

    TabList_clear(app.tabs);
exit_db:

    return ret;
}
