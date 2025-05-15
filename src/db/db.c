#include "db.h"
#include "path/path.h"

#include <sqlite3/sqlite3.h>

struct Db {
    sqlite3* conn;
};

static void db_perror(Db* db, const char* s) {
    custom_perror(s, sqlite3_errmsg(db->conn));
}

Db* db_init(void) {
    Db* db = malloc(sizeof(Db));

    Path* path = path_init_data_dir();
    path_join(path, "temp.sqlite3"); // TODO: change when its ready
    int32_t res = sqlite3_open(path_cstr(path), &db->conn);
    path_free(path);

    if(res != SQLITE_OK) {
        db_perror(db, "sqlite3_open()");
        sqlite3_close(db->conn);
        free(db);
        return NULL;
    }

    return db;
}

void db_free(Db* db) {
    sqlite3_close(db->conn);
    free(db);
}
