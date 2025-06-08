#pragma once

#include "db/db.h"
#include "game/game.h"
#include "gui/gui.h"
#include "settings/settings.h"
#include "types/label.h"
#include "types/tab.h"

extern Os os;
extern Db* db;
extern Gui* gui;
extern TabList tabs;
extern LabelList labels;
extern Settings* settings;
extern GameDict games;
