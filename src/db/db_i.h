#pragma once

#include "db.h"

#include <dcimgui/dcimgui.h>
#include <sqlite3/sqlite3.h>
#include <std.h>

typedef struct {
    enum {
        DbMessageType_Quit,
        DbMessageType_Backup,

        DbMessageType_LoadSettings,
        DbMessageType_SaveSetting,

        DbMessageType_LoadTabs,
        DbMessageType_SaveTab,
        DbMessageType_CreateTab,
        DbMessageType_DeleteTab,

        DbMessageType_LoadLabels,
        DbMessageType_SaveLabel,
        DbMessageType_CreateLabel,
        DbMessageType_DeleteLabel,
    } type;
    m_eflag_t* eflag;
    union {
        union {
            Settings* settings;
            TabList_t* tabs;
            LabelList_t* labels;
        } load;
        union {
            struct {
                const Settings* ptr;
                SettingsColumn column;
            } setting;
            struct {
                const Tab* ptr;
                TabsColumn column;
            } tab;
            struct {
                const Label* ptr;
                LabelsColumn column;
            } label;
        } save;
        union {
            struct {
                TabList_t* tabs;
                Tab** out;
            } tab;
            struct {
                LabelList_t* labels;
                Label** out;
            } label;
        } create;
        union {
            struct {
                const Tab* ptr;
                TabList_t* tabs;
            } tab;
            struct {
                const Label* ptr;
                LabelList_t* labels;
            } label;
        } delete;
    };
} DbMessage;

// TODO: check if 100 queue is really useful
M_BUFFER_DEF(DbMessageQueue, DbMessage, 100, M_BUFFER_QUEUE, M_POD_OPLIST)

struct Db {
    Path* path;
    sqlite3* conn;
    const char* name;
    m_thread_t thread;
    DbMessageQueue_t queue;
    bool did_migration_backup;
};

static inline void db_perror(Db* db, const char* s) {
    custom_perror(s, sqlite3_errmsg(db->conn));
}

#define db_assert(db, res, exp, s) \
    if(res != exp) {               \
        db_perror(db, s);          \
    }                              \
    assert(res == exp)

void db_create_table(Db* db, const DbTable* table);
void db_append_column_names(m_string_t* sql, const DbTable* table);

void db_do_load_settings(Db* db, Settings* settings);
void db_do_save_setting(Db* db, const Settings* settings, SettingsColumn column);

void db_do_load_tabs(Db* db, TabList_t* tabs);
void db_do_save_tab(Db* db, const Tab* tab, TabsColumn column);
Tab* db_do_create_tab(Db* db, TabList_t* tabs);
void db_do_delete_tab(Db* db, const Tab* tab, TabList_t* tabs);

void db_do_load_labels(Db* db, LabelList_t* labels);
void db_do_save_label(Db* db, const Label* label, LabelsColumn column);
Label* db_do_create_label(Db* db, LabelList_t* labels);
void db_do_delete_label(Db* db, const Label* label, LabelList_t* labels);

#define sqlite3_column_count(pStmt)   (size_t)sqlite3_column_count(pStmt)
#define sqlite3_column_text(pStmt, i) (const char*)sqlite3_column_text(pStmt, i)
#define sqlite3_column_json(pStmt, i) json_tokener_parse(sqlite3_column_text(pStmt, i))
ImColor sqlite3_column_imcolor(sqlite3_stmt* stmt, int32_t col);
#define sqlite3_bind_mstring(pStmt, i, str) \
    sqlite3_bind_text(pStmt, i, m_string_get_cstr(str), -1, SQLITE_TRANSIENT)
#define sqlite3_bind_json(pStmt, i, json)                             \
    sqlite3_bind_text(                                                \
        pStmt,                                                        \
        i,                                                            \
        json_object_to_json_string_ext(                               \
            json,                                                     \
            JSON_C_TO_STRING_PLAIN | JSON_C_TO_STRING_NOSLASHESCAPE), \
        -1,                                                           \
        SQLITE_TRANSIENT)
#define json_object_object_add_unique(obj, key, val) \
    json_object_object_add_ex(obj, key, val, JSON_C_OBJECT_ADD_KEY_IS_NEW)
int32_t sqlite3_bind_imcolor(sqlite3_stmt* stmt, int32_t param, ImColor im_color);
