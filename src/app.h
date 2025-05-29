#pragma once

#include "db/db.h"
#include "game/game.h"
#include "gui/gui.h"
#include "settings/settings.h"
#include "types/label.h"
#include "types/tab.h"

typedef struct {
    const char* version;
    Os os;
    Db* db;
    Gui* gui;
    TabList_t tabs;
    LabelList_t labels;
    Settings* settings;
    GameDict_t games;
} App;

extern App app;
