#pragma once

#include "games.h"
#include "labels.h"
#include "settings.h"
#include "tabs.h"

#include "game/game.h"
#include "settings/settings.h"
#include "types/label.h"
#include "types/tab.h"

#include <std.h>

typedef struct Db Db;

Db* db_init(void);

void db_backup(Db* db);

void db_load_settings(Db* db, Settings* settings);
void db_save_setting(Db* db, const Settings* settings, SettingsColumn column);

void db_load_games(Db* db, GameDict_t* games);
void db_save_game(Db* db, const Game* game, GamesColumn column);
Game* db_create_game(Db* db, GameDict_t* games, GameId id);
void db_delete_game(Db* db, Game* game, GameDict_t* games);

void db_load_tabs(Db* db, TabList_t* tabs);
void db_save_tab(Db* db, const Tab* tab, TabsColumn column);
Tab* db_create_tab(Db* db, TabList_t* tabs);
void db_delete_tab(Db* db, Tab* tab, TabList_t* tabs);

void db_load_labels(Db* db, LabelList_t* labels);
void db_save_label(Db* db, const Label* label, LabelsColumn column);
Label* db_create_label(Db* db, LabelList_t* labels);
void db_delete_label(Db* db, Label* label, LabelList_t* labels);

void db_free(Db* db);
