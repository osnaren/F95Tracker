#pragma once

#include "db/db.h"
#include "gui/gui.h"
#include "settings/settings.h"

typedef struct {
    const char* version;
    Os os;
    Db* db;
    Gui* gui;
    Settings* settings;
} App;

extern App app;
