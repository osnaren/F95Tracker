#pragma once

#include "settings.h"
#include "tabs.h"

#include "settings/settings.h"
#include "types/tab.h"

#include <std.h>

typedef struct Db Db;

Db* db_init(void);
void db_backup(Db* db);
void db_load_settings(Db* db, Settings* settings);
void db_save_setting(Db* db, const Settings* settings, SettingsColumn column);
void db_load_tabs(Db* db, TabList_t* tabs);
void db_save_tab(Db* db, const Tab* tab, TabsColumn column);
void db_free(Db* db);
