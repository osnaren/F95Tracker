#pragma once

#include "settings/settings.h"

#include <std.h>

typedef struct Db Db;

Db* db_init(void);
void db_load_settings(Db* db, Settings* settings);
void db_free(Db* db);
