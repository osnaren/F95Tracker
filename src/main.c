#include <globals.h>

#if OS == OS_WINDOWS
Os os = Os_Windows;
#elif OS == OS_LINUX
Os os = Os_Linux;
#elif OS == OS_MACOS
Os os = Os_MacOS;
#endif
Db* db;
Gui* gui;
TabList tabs;
LabelList labels;
Settings* settings;
GameDict games;

int32_t main(int32_t argc, char** argv) {
    UNUSED(argc);
    UNUSED(argv);
    int32_t ret = 0;

    db = db_init();
    if(db == NULL) {
        ret = 1;
        goto exit_db;
    }

    tab_list_init(tabs);
    db_load_tabs(db, tabs);

    label_list_init(labels);
    db_load_labels(db, labels);

    settings = settings_init();
    db_load_settings(db, settings);

    game_dict_init(games);
    db_load_games(db, games);

    gui = gui_init();
    if(gui == NULL) {
        ret = 1;
        goto exit_gui;
    }

    while(!gui_should_close(gui)) {
        gui_tick(gui);
    }

    gui_free(gui);
exit_gui:

    db_free(db);

    for each(GameDict_pair, pair, GameDict, games) {
        game_free(pair.value);
    }
    game_dict_clear(games);

    settings_free(settings);

    label_list_clear(labels);

    tab_list_clear(tabs);
exit_db:

    return ret;
}
