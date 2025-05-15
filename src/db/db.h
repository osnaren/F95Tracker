#pragma once

#include <std.h>

typedef struct Db Db;

Db* db_init(void);
void db_free(Db* db);
