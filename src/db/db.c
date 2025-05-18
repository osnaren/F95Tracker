#include "db.h"
#include "app.h"
#include "path/path.h"

#include <sqlite3/sqlite3.h>

#define sqlite3_column_count(pStmt)   (size_t)sqlite3_column_count(pStmt)
#define sqlite3_column_text(pStmt, i) (const char*)sqlite3_column_text(pStmt, i)
#define sqlite3_bind_mstring(pStmt, i, str) \
    sqlite3_bind_text(pStmt, i, m_string_get_cstr(str), -1, SQLITE_TRANSIENT)

#define DB_FILE "temp.sqlite3" // TODO: change when its ready

typedef struct {
    enum {
        DbMessageType_Quit,
        DbMessageType_Backup,
        DbMessageType_SaveSettings,
    } type;
    union {
        struct {
            const Settings* ptr;
            SettingsColumn column;
        } save_settings;
    };
} DbMessage;

M_BUFFER_DEF(DbMessageQueue, DbMessage, 100, M_BUFFER_QUEUE, M_POD_OPLIST)

struct Db {
    Path* path;
    sqlite3* conn;
    const char* name;
    m_thread_t thread;
    DbMessageQueue_t queue;
};

typedef struct {
    const char* name;
    const char* type;
    const char* dflt;
    bool primary_key;
    bool autoincrement;
    const char* extra;
} DbColumn;

typedef struct {
    const char* old;
    const char* new;
} DbRename;

typedef struct {
    const char* name;
    const DbColumn* columns;
    size_t columns_count;
    const DbRename* renames;
    size_t renames_count;
} DbTable;

#define DB_COLUMN_ITEM(enum_name, column, ...)    [enum_name##_##column] = {#column, __VA_ARGS__},
#define DB_RENAME_ITEM(empty, old_name, new_name) {.old = #old_name, .new = #new_name},
#define DB_TABLE_DEFINE(TABLE_MACRO, table_name, enum_name) \
    static const DbColumn _##table_name##_columns[] = {     \
        TABLE_MACRO(enum_name, DB_COLUMN_ITEM, DB_EMPTY)};  \
    static const DbRename _##table_name##_renames[] = {     \
        TABLE_MACRO(DB_EMPTY, DB_EMPTY, DB_RENAME_ITEM)};   \
    static const DbTable table_name##_table = {             \
        .name = #table_name,                                \
        .columns = _##table_name##_columns,                 \
        .columns_count = COUNT_OF(_##table_name##_columns), \
        .renames = _##table_name##_renames,                 \
        .renames_count = COUNT_OF(_##table_name##_renames), \
    };
DB_TABLE_DEFINE(_SETTINGS, settings, SettingsColumn)

static void db_perror(Db* db, const char* s) {
    custom_perror(s, sqlite3_errmsg(db->conn));
}

#define db_assert(db, res, exp, s) \
    if(res != exp) {               \
        db_perror(db, s);          \
    }                              \
    assert(res == exp)

void db_thread(void* ctx);

Db* db_init(void) {
    Db* db = malloc(sizeof(Db));
    int32_t res;

    db->path = path_init_data_dir();
    path_join(db->path, DB_FILE);
    res = sqlite3_open(path_cstr(db->path), &db->conn);

    if(res != SQLITE_OK) {
        db_perror(db, "sqlite3_open()");
        sqlite3_close(db->conn);
        path_free(db->path);
        free(db);
        return NULL;
    }

    // TODO: make note somewhere that legacy json and ini config (pre v9.0) are no longer migrated,
    // need v11.x to migrate to sqlite3 from those legacy configs, then this will work fine

    db->name = strdup(sqlite3_db_name(db->conn, 0));

    // Use WAL mode for more efficient writes
    res = sqlite3_exec(db->conn, "PRAGMA journal_mode=WAL", NULL, NULL, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

    // Consolidate WAL into DB every 100 frames, usually means ~100 write operations
    res = sqlite3_wal_autocheckpoint(db->conn, 100);
    db_assert(db, res, SQLITE_OK, "sqlite3_wal_autocheckpoint()");

    // FIXME: move table loading to thread
    DbMessageQueue_init(db->queue, 100);
    m_thread_create(db->thread, db_thread, db);

    return db;
}

void db_backup(Db* db) {
    const DbMessage message = {
        .type = DbMessageType_Backup,
    };
    DbMessageQueue_push(db->queue, message);
}

static void db_append_backup_id(m_string_t* str) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    struct tm* tp = localtime(&ts.tv_sec);
    m_string_t id_str;
    m_string_init_printf(
        id_str,
        "_v%s_%04d%02d%02d_%02d%02d%02d_%d",
        app.version,
        tp->tm_year + 1900,
        tp->tm_mon + 1,
        tp->tm_mday,
        tp->tm_hour,
        tp->tm_min,
        tp->tm_sec,
        ts.tv_nsec);
    m_string_replace_all_cstr(id_str, ".", "");
    m_string_cat(*str, id_str);
    m_string_clear(id_str);
}

static void db_do_backup(Db* db) {
    int32_t res;

    res = sqlite3_wal_checkpoint_v2(db->conn, db->name, SQLITE_CHECKPOINT_TRUNCATE, NULL, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_wal_checkpoint_v2()");

    Path* backup = path_dup(db->path);
    m_string_t name;
    m_string_init_set(name, path_name(backup));
    db_append_backup_id(&name);
    path_set_name(backup, m_string_get_cstr(name));
    m_string_clear(name);

    m_bstring_t bytes;
    m_bstring_init(bytes);
    if(path_read(db->path, &bytes) && path_write(backup, &bytes)) {
        printf("Saved DB backup: %s\n", path_cstr(backup));
    } else {
        custom_perror(path_cstr(backup), "DB backup failed");
    }
    m_bstring_clear(bytes);

    path_free(backup);
}

static void db_append_column_spec(m_string_t* sql, const DbColumn* column) {
    m_string_cat_printf(*sql, "%s %s", column->name, column->type);
    if(column->dflt) {
        m_string_cat_printf(*sql, " DEFAULT %s", column->dflt);
    }
    if(column->primary_key) {
        m_string_cat(*sql, " PRIMARY KEY");
        if(column->autoincrement) {
            m_string_cat(*sql, " AUTOINCREMENT");
        }
    }
    if(column->extra) {
        m_string_cat_printf(*sql, " %s", column->extra);
    }
}

static void db_append_column_names(m_string_t* sql, const DbTable* table) {
    for(size_t col = 0; col < table->columns_count; col++) {
        m_string_cat(*sql, table->columns[col].name);
        m_string_cat(*sql, ",");
    }
    m_string_strim(*sql, ",");
}

static void db_create_table(Db* db, const DbTable* table) {
    int32_t res;
    m_string_t sql;
    m_string_init(sql);
    sqlite3_stmt* stmt;

    // Create table
    m_string_printf(sql, "CREATE TABLE IF NOT EXISTS %s (", table->name);
    for(size_t col = 0; col < table->columns_count; col++) {
        const DbColumn* column = &table->columns[col];
        db_append_column_spec(&sql, column);
        m_string_cat(sql, ",");
    }
    m_string_strim(sql, ",");
    m_string_cat(sql, ")");

    res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

    // TODO: make backup if migration is needed

    // Rename columns
    for(size_t ren = 0; ren < table->renames_count; ren++) {
        const DbRename* rename = &table->renames[ren];
        res = sqlite3_table_column_metadata(
            db->conn,
            db->name,
            table->name,
            rename->old,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL);
        if(res == SQLITE_ERROR) {
            continue; // Doesn't exist, nothing to rename
        }

        m_string_printf(
            sql,
            "ALTER TABLE %s RENAME COLUMN %s TO %s",
            table->name,
            rename->old,
            rename->new);
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");
    }

    // Add columns
    for(size_t col = 0; col < table->columns_count; col++) {
        const DbColumn* column = &table->columns[col];
        res = sqlite3_table_column_metadata(
            db->conn,
            db->name,
            table->name,
            column->name,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL);
        if(res == SQLITE_OK) {
            continue; // Already exists, nothing to add
        }

        m_string_printf(sql, "ALTER TABLE %s ADD COLUMN ", table->name);
        db_append_column_spec(&sql, column);
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");
    }

    // Remove columns
    m_string_printf(sql, "PRAGMA table_info(%s)", table->name);
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    while((res = sqlite3_step(stmt)) != SQLITE_DONE) {
        db_assert(db, res, SQLITE_ROW, "sqlite3_step()");
        const char* found_column = sqlite3_column_text(stmt, 1);
        bool is_known = false;
        for(size_t col = 0; col < table->columns_count; col++) {
            const DbColumn* known_column = &table->columns[col];
            if(strcmp(known_column->name, found_column) == 0) {
                is_known = true;
                break;
            }
        }
        if(is_known) {
            continue; // Should exist, nothing to remove
        }

        m_string_printf(sql, "ALTER TABLE %s DROP COLUMN %s", table->name, found_column);
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");
    }

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    // Update column defs
    m_string_printf(sql, "PRAGMA table_info(%s)", table->name);
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    bool recreate = false;
    while((res = sqlite3_step(stmt)) != SQLITE_DONE) {
        db_assert(db, res, SQLITE_ROW, "sqlite3_step()");
        const char* found_column = sqlite3_column_text(stmt, 1);
        const DbColumn* column = NULL;
        for(size_t col = 0; col < table->columns_count; col++) {
            const DbColumn* known_column = &table->columns[col];
            if(strcmp(known_column->name, found_column) == 0) {
                column = known_column;
                break;
            }
        }
        assert(column != NULL);

        const char* found_type = sqlite3_column_text(stmt, 2);
        const char* found_dflt = sqlite3_column_text(stmt, 4);
        bool changed_type = (column->type == NULL) != (found_type == NULL);
        bool changed_dflt = (column->dflt == NULL) != (found_dflt == NULL);
        if(column->type != NULL && found_type != NULL) {
            changed_type = strcasecmp(column->type, found_type) != 0;
        }
        if(column->dflt != NULL && found_dflt != NULL) {
            changed_type = strcasecmp(column->dflt, found_dflt) != 0;
        }
        bool changed_primary_key = column->primary_key != (sqlite3_column_int(stmt, 5) != 0);

        if(changed_type || changed_dflt || changed_primary_key) {
            recreate = true;
            break;
        }
    }

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    if(recreate) {
        m_string_t table_temp;
        m_string_init_printf(table_temp, "%s_temp", table->name);
        db_append_backup_id(&table_temp);

        m_string_printf(
            sql,
            "ALTER TABLE %s RENAME TO %s",
            table->name,
            m_string_get_cstr(table_temp));
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

        db_create_table(db, table);

        m_string_printf(sql, "INSERT INTO %s (", table->name);
        db_append_column_names(&sql, table);
        m_string_cat(sql, ") SELECT ");
        db_append_column_names(&sql, table);
        m_string_cat_printf(sql, " FROM %s", m_string_get_cstr(table_temp));
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

        m_string_printf(sql, "DROP TABLE %s", m_string_get_cstr(table_temp));
        res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
        db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

        m_string_clear(table_temp);
    }

    m_string_clear(sql);
}

void db_load_settings(Db* db, Settings* settings) {
    int32_t res;
    m_string_t sql;
    m_string_init(sql);

    // Create the table and handle schema migrations
    db_create_table(db, &settings_table);

    // Insert the main settings row
    m_string_printf(
        sql,
        "INSERT INTO %s (_) VALUES (0) ON CONFLICT DO NOTHING",
        settings_table.name);
    res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

    // Read the main settings row
    m_string_set(sql, "SELECT ");
    db_append_column_names(&sql, &settings_table);
    m_string_cat_printf(sql, " FROM %s", settings_table.name);

    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_ROW, "sqlite3_step()");

    assert(sqlite3_column_count(stmt) == settings_table.columns_count);
    size_t col = 1; // Skip _ column
    settings->background_on_close = sqlite3_column_int(stmt, col++);
    settings->bg_notifs_interval = sqlite3_column_int(stmt, col++);
    settings->bg_refresh_interval = sqlite3_column_int(stmt, col++);
    m_string_set(settings->browser_custom_arguments, sqlite3_column_text(stmt, col++));
    m_string_set(settings->browser_custom_executable, sqlite3_column_text(stmt, col++));
    settings->browser_html = sqlite3_column_int(stmt, col++);
    settings->browser_private = sqlite3_column_int(stmt, col++);
    // settings->browser = sqlite3_column_int(stmt, column_i++);
    col++;
    settings->cell_image_ratio = sqlite3_column_double(stmt, col++);
    settings->check_notifs = sqlite3_column_int(stmt, col++);
    settings->compact_timeline = sqlite3_column_int(stmt, col++);
    settings->confirm_on_remove = sqlite3_column_int(stmt, col++);
    settings->copy_urls_as_bbcode = sqlite3_column_int(stmt, col++);
    m_string_set(settings->datestamp_format, sqlite3_column_text(stmt, col++));
    // settings->default_exe_dir = sqlite3_column_int(stmt, column_i++);
    col++;
    settings->default_tab_is_new = sqlite3_column_int(stmt, col++);
    settings->display_mode = sqlite3_column_int(stmt, col++);
    // settings->display_tab = sqlite3_column_int(stmt, column_i++);
    col++;
    // settings->downloads_dir = sqlite3_column_int(stmt, column_i++);
    col++;
    settings->ext_background_add = sqlite3_column_int(stmt, col++);
    settings->ext_highlight_tags = sqlite3_column_int(stmt, col++);
    settings->ext_icon_glow = sqlite3_column_int(stmt, col++);
    settings->filter_all_tabs = sqlite3_column_int(stmt, col++);
    settings->fit_images = sqlite3_column_int(stmt, col++);
    settings->grid_columns = sqlite3_column_int(stmt, col++);
    // settings->hidden_timeline_events = sqlite3_column_int(stmt, column_i++);
    col++;
    settings->hide_empty_tabs = sqlite3_column_int(stmt, col++);
    settings->highlight_tags = sqlite3_column_int(stmt, col++);
    settings->ignore_semaphore_timeouts = sqlite3_column_int(stmt, col++);
    settings->independent_tab_views = sqlite3_column_int(stmt, col++);
    settings->insecure_ssl = sqlite3_column_int(stmt, col++);
    settings->interface_scaling = sqlite3_column_double(stmt, col++);
    // settings->last_successful_refresh = sqlite3_column_int(stmt, column_i++);
    col++;
    // settings->manual_sort_list = sqlite3_column_int(stmt, column_i++);
    col++;
    settings->mark_installed_after_add = sqlite3_column_int(stmt, col++);
    settings->max_connections = sqlite3_column_int(stmt, col++);
    settings->max_retries = sqlite3_column_int(stmt, col++);
    settings->notifs_show_update_banner = sqlite3_column_int(stmt, col++);
    settings->play_gifs = sqlite3_column_int(stmt, col++);
    settings->play_gifs_unfocused = sqlite3_column_int(stmt, col++);
    settings->preload_nearby_images = sqlite3_column_int(stmt, col++);
    settings->proxy_type = sqlite3_column_int(stmt, col++);
    m_string_set(settings->proxy_host, sqlite3_column_text(stmt, col++));
    settings->proxy_port = sqlite3_column_int(stmt, col++);
    m_string_set(settings->proxy_username, sqlite3_column_text(stmt, col++));
    m_string_set(settings->proxy_password, sqlite3_column_text(stmt, col++));
    settings->quick_filters = sqlite3_column_int(stmt, col++);
    settings->refresh_archived_games = sqlite3_column_int(stmt, col++);
    settings->refresh_completed_games = sqlite3_column_int(stmt, col++);
    settings->render_when_unfocused = sqlite3_column_int(stmt, col++);
    settings->request_timeout = sqlite3_column_int(stmt, col++);
    settings->rpc_enabled = sqlite3_column_int(stmt, col++);
    m_string_set(settings->rpdl_password, sqlite3_column_text(stmt, col++));
    m_string_set(settings->rpdl_token, sqlite3_column_text(stmt, col++));
    m_string_set(settings->rpdl_username, sqlite3_column_text(stmt, col++));
    settings->scroll_amount = sqlite3_column_int(stmt, col++);
    settings->scroll_smooth = sqlite3_column_int(stmt, col++);
    settings->scroll_smooth_speed = sqlite3_column_int(stmt, col++);
    settings->select_executable_after_add = sqlite3_column_int(stmt, col++);
    settings->show_remove_btn = sqlite3_column_int(stmt, col++);
    settings->software_webview = sqlite3_column_int(stmt, col++);
    settings->start_in_background = sqlite3_column_int(stmt, col++);
    settings->start_refresh = sqlite3_column_int(stmt, col++);
    // settings->style_accent = sqlite3_column_int(stmt, column_i++);
    col++;
    // settings->style_alt_bg = sqlite3_column_int(stmt, column_i++);
    col++;
    // settings->style_bg = sqlite3_column_int(stmt, column_i++);
    col++;
    // settings->style_border = sqlite3_column_int(stmt, column_i++);
    col++;
    settings->style_corner_radius = sqlite3_column_int(stmt, col++);
    // settings->style_text = sqlite3_column_int(stmt, column_i++);
    col++;
    // settings->style_text_dim = sqlite3_column_int(stmt, column_i++);
    col++;
    settings->table_header_outside_list = sqlite3_column_int(stmt, col++);
    // settings->tags_highlights = sqlite3_column_int(stmt, column_i++);
    col++;
    settings->tex_compress = sqlite3_column_int(stmt, col++);
    settings->tex_compress_replace = sqlite3_column_int(stmt, col++);
    m_string_set(settings->timestamp_format, sqlite3_column_text(stmt, col++));
    settings->unload_offscreen_images = sqlite3_column_int(stmt, col++);
    settings->vsync_ratio = sqlite3_column_int(stmt, col++);
    settings->weighted_score = sqlite3_column_int(stmt, col++);
    settings->zoom_area = sqlite3_column_int(stmt, col++);
    settings->zoom_enabled = sqlite3_column_int(stmt, col++);
    settings->zoom_times = sqlite3_column_int(stmt, col++);

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);

    // def __post_init__(self):
    //     if "" in self.default_exe_dir:
    //     from modules import globals
    //     self.default_exe_dir[globals.os] = self.default_exe_dir[""]
    //     del self.default_exe_dir[""]
}

void db_save_settings(Db* db, const Settings* settings, SettingsColumn column) {
    const DbMessage message = {
        .type = DbMessageType_SaveSettings,
        .save_settings =
            {
                .ptr = settings,
                .column = column,
            },
    };
    DbMessageQueue_push(db->queue, message);
}

static void db_do_save_settings(Db* db, const Settings* settings, SettingsColumn column) {
    int32_t res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(
        sql,
        "UPDATE %s SET %s=? WHERE _=0",
        settings_table.name,
        settings_table.columns[column].name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    switch(column) {
    case SettingsColumn__:
        res = sqlite3_bind_int(stmt, 1, 0);
        break;
    case SettingsColumn_background_on_close:
        res = sqlite3_bind_int(stmt, 1, settings->background_on_close);
        break;
    case SettingsColumn_bg_notifs_interval:
        res = sqlite3_bind_int(stmt, 1, settings->bg_notifs_interval);
        break;
    case SettingsColumn_bg_refresh_interval:
        res = sqlite3_bind_int(stmt, 1, settings->bg_refresh_interval);
        break;
    case SettingsColumn_browser_custom_arguments:
        res = sqlite3_bind_mstring(stmt, 1, settings->browser_custom_arguments);
        break;
    case SettingsColumn_browser_custom_executable:
        res = sqlite3_bind_mstring(stmt, 1, settings->browser_custom_executable);
        break;
    case SettingsColumn_browser_html:
        res = sqlite3_bind_int(stmt, 1, settings->browser_html);
        break;
    case SettingsColumn_browser_private:
        res = sqlite3_bind_int(stmt, 1, settings->browser_private);
        break;
    case SettingsColumn_browser:
        // res = sqlite3_bind_int(stmt, 1, settings->browser);
        break;
    case SettingsColumn_cell_image_ratio:
        res = sqlite3_bind_int(stmt, 1, settings->cell_image_ratio);
        break;
    case SettingsColumn_check_notifs:
        res = sqlite3_bind_int(stmt, 1, settings->check_notifs);
        break;
    case SettingsColumn_compact_timeline:
        res = sqlite3_bind_int(stmt, 1, settings->compact_timeline);
        break;
    case SettingsColumn_confirm_on_remove:
        res = sqlite3_bind_int(stmt, 1, settings->confirm_on_remove);
        break;
    case SettingsColumn_copy_urls_as_bbcode:
        res = sqlite3_bind_int(stmt, 1, settings->copy_urls_as_bbcode);
        break;
    case SettingsColumn_datestamp_format:
        res = sqlite3_bind_mstring(stmt, 1, settings->datestamp_format);
        break;
    case SettingsColumn_default_exe_dir:
        // res = sqlite3_bind_int(stmt, 1, settings->default_exe_dir);
        break;
    case SettingsColumn_default_tab_is_new:
        res = sqlite3_bind_int(stmt, 1, settings->default_tab_is_new);
        break;
    case SettingsColumn_display_mode:
        res = sqlite3_bind_int(stmt, 1, settings->display_mode);
        break;
    case SettingsColumn_display_tab:
        // res = sqlite3_bind_int(stmt, 1, settings->display_tab);
        break;
    case SettingsColumn_downloads_dir:
        // res = sqlite3_bind_int(stmt, 1, settings->downloads_dir);
        break;
    case SettingsColumn_ext_background_add:
        res = sqlite3_bind_int(stmt, 1, settings->ext_background_add);
        break;
    case SettingsColumn_ext_highlight_tags:
        res = sqlite3_bind_int(stmt, 1, settings->ext_highlight_tags);
        break;
    case SettingsColumn_ext_icon_glow:
        res = sqlite3_bind_int(stmt, 1, settings->ext_icon_glow);
        break;
    case SettingsColumn_filter_all_tabs:
        res = sqlite3_bind_int(stmt, 1, settings->filter_all_tabs);
        break;
    case SettingsColumn_fit_images:
        res = sqlite3_bind_int(stmt, 1, settings->fit_images);
        break;
    case SettingsColumn_grid_columns:
        res = sqlite3_bind_int(stmt, 1, settings->grid_columns);
        break;
    case SettingsColumn_hidden_timeline_events:
        // res = sqlite3_bind_int(stmt, 1, settings->hidden_timeline_events);
        break;
    case SettingsColumn_hide_empty_tabs:
        res = sqlite3_bind_int(stmt, 1, settings->hide_empty_tabs);
        break;
    case SettingsColumn_highlight_tags:
        res = sqlite3_bind_int(stmt, 1, settings->highlight_tags);
        break;
    case SettingsColumn_ignore_semaphore_timeouts:
        res = sqlite3_bind_int(stmt, 1, settings->ignore_semaphore_timeouts);
        break;
    case SettingsColumn_independent_tab_views:
        res = sqlite3_bind_int(stmt, 1, settings->independent_tab_views);
        break;
    case SettingsColumn_insecure_ssl:
        res = sqlite3_bind_int(stmt, 1, settings->insecure_ssl);
        break;
    case SettingsColumn_interface_scaling:
        res = sqlite3_bind_int(stmt, 1, settings->interface_scaling);
        break;
    case SettingsColumn_last_successful_refresh:
        // res = sqlite3_bind_int(stmt, 1, settings->last_successful_refresh);
        break;
    case SettingsColumn_manual_sort_list:
        // res = sqlite3_bind_int(stmt, 1, settings->manual_sort_list);
        break;
    case SettingsColumn_mark_installed_after_add:
        res = sqlite3_bind_int(stmt, 1, settings->mark_installed_after_add);
        break;
    case SettingsColumn_max_connections:
        res = sqlite3_bind_int(stmt, 1, settings->max_connections);
        break;
    case SettingsColumn_max_retries:
        res = sqlite3_bind_int(stmt, 1, settings->max_retries);
        break;
    case SettingsColumn_notifs_show_update_banner:
        res = sqlite3_bind_int(stmt, 1, settings->notifs_show_update_banner);
        break;
    case SettingsColumn_play_gifs:
        res = sqlite3_bind_int(stmt, 1, settings->play_gifs);
        break;
    case SettingsColumn_play_gifs_unfocused:
        res = sqlite3_bind_int(stmt, 1, settings->play_gifs_unfocused);
        break;
    case SettingsColumn_preload_nearby_images:
        res = sqlite3_bind_int(stmt, 1, settings->preload_nearby_images);
        break;
    case SettingsColumn_proxy_type:
        res = sqlite3_bind_int(stmt, 1, settings->proxy_type);
        break;
    case SettingsColumn_proxy_host:
        res = sqlite3_bind_mstring(stmt, 1, settings->proxy_host);
        break;
    case SettingsColumn_proxy_port:
        res = sqlite3_bind_int(stmt, 1, settings->proxy_port);
        break;
    case SettingsColumn_proxy_username:
        res = sqlite3_bind_mstring(stmt, 1, settings->proxy_username);
        break;
    case SettingsColumn_proxy_password:
        res = sqlite3_bind_mstring(stmt, 1, settings->proxy_password);
        break;
    case SettingsColumn_quick_filters:
        res = sqlite3_bind_int(stmt, 1, settings->quick_filters);
        break;
    case SettingsColumn_refresh_archived_games:
        res = sqlite3_bind_int(stmt, 1, settings->refresh_archived_games);
        break;
    case SettingsColumn_refresh_completed_games:
        res = sqlite3_bind_int(stmt, 1, settings->refresh_completed_games);
        break;
    case SettingsColumn_render_when_unfocused:
        res = sqlite3_bind_int(stmt, 1, settings->render_when_unfocused);
        break;
    case SettingsColumn_request_timeout:
        res = sqlite3_bind_int(stmt, 1, settings->request_timeout);
        break;
    case SettingsColumn_rpc_enabled:
        res = sqlite3_bind_int(stmt, 1, settings->rpc_enabled);
        break;
    case SettingsColumn_rpdl_password:
        res = sqlite3_bind_mstring(stmt, 1, settings->rpdl_password);
        break;
    case SettingsColumn_rpdl_token:
        res = sqlite3_bind_mstring(stmt, 1, settings->rpdl_token);
        break;
    case SettingsColumn_rpdl_username:
        res = sqlite3_bind_mstring(stmt, 1, settings->rpdl_username);
        break;
    case SettingsColumn_scroll_amount:
        res = sqlite3_bind_int(stmt, 1, settings->scroll_amount);
        break;
    case SettingsColumn_scroll_smooth:
        res = sqlite3_bind_int(stmt, 1, settings->scroll_smooth);
        break;
    case SettingsColumn_scroll_smooth_speed:
        res = sqlite3_bind_int(stmt, 1, settings->scroll_smooth_speed);
        break;
    case SettingsColumn_select_executable_after_add:
        res = sqlite3_bind_int(stmt, 1, settings->select_executable_after_add);
        break;
    case SettingsColumn_show_remove_btn:
        res = sqlite3_bind_int(stmt, 1, settings->show_remove_btn);
        break;
    case SettingsColumn_software_webview:
        res = sqlite3_bind_int(stmt, 1, settings->software_webview);
        break;
    case SettingsColumn_start_in_background:
        res = sqlite3_bind_int(stmt, 1, settings->start_in_background);
        break;
    case SettingsColumn_start_refresh:
        res = sqlite3_bind_int(stmt, 1, settings->start_refresh);
        break;
    case SettingsColumn_style_accent:
        // res = sqlite3_bind_int(stmt, 1, settings->style_accent);
        break;
    case SettingsColumn_style_alt_bg:
        // res = sqlite3_bind_int(stmt, 1, settings->style_alt_bg);
        break;
    case SettingsColumn_style_bg:
        // res = sqlite3_bind_int(stmt, 1, settings->style_bg);
        break;
    case SettingsColumn_style_border:
        // res = sqlite3_bind_int(stmt, 1, settings->style_border);
        break;
    case SettingsColumn_style_corner_radius:
        res = sqlite3_bind_int(stmt, 1, settings->style_corner_radius);
        break;
    case SettingsColumn_style_text:
        // res = sqlite3_bind_int(stmt, 1, settings->style_text);
        break;
    case SettingsColumn_style_text_dim:
        // res = sqlite3_bind_int(stmt, 1, settings->style_text_dim);
        break;
    case SettingsColumn_table_header_outside_list:
        res = sqlite3_bind_int(stmt, 1, settings->table_header_outside_list);
        break;
    case SettingsColumn_tags_highlights:
        // res = sqlite3_bind_int(stmt, 1, settings->tags_highlights);
        break;
    case SettingsColumn_tex_compress:
        res = sqlite3_bind_int(stmt, 1, settings->tex_compress);
        break;
    case SettingsColumn_tex_compress_replace:
        res = sqlite3_bind_int(stmt, 1, settings->tex_compress_replace);
        break;
    case SettingsColumn_timestamp_format:
        res = sqlite3_bind_mstring(stmt, 1, settings->timestamp_format);
        break;
    case SettingsColumn_unload_offscreen_images:
        res = sqlite3_bind_int(stmt, 1, settings->unload_offscreen_images);
        break;
    case SettingsColumn_vsync_ratio:
        res = sqlite3_bind_int(stmt, 1, settings->vsync_ratio);
        break;
    case SettingsColumn_weighted_score:
        res = sqlite3_bind_int(stmt, 1, settings->weighted_score);
        break;
    case SettingsColumn_zoom_area:
        res = sqlite3_bind_int(stmt, 1, settings->zoom_area);
        break;
    case SettingsColumn_zoom_enabled:
        res = sqlite3_bind_int(stmt, 1, settings->zoom_enabled);
        break;
    case SettingsColumn_zoom_times:
        res = sqlite3_bind_int(stmt, 1, settings->zoom_times);
        break;
    }
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_*()");

    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_DONE, "sqlite3_step()");

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
}

void db_thread(void* ctx) {
    Db* db = ctx;
    bool quit = false;

    DbMessage message;
    while(DbMessageQueue_pop_blocking(&message, db->queue, !quit)) {
        switch(message.type) {
        case DbMessageType_Quit:
            quit = true;
            break;
        case DbMessageType_Backup:
            db_do_backup(db);
            break;
        case DbMessageType_SaveSettings:
            db_do_save_settings(db, message.save_settings.ptr, message.save_settings.column);
            break;
        }
    }
}

void db_free(Db* db) {
    const DbMessage message = {
        .type = DbMessageType_Quit,
    };
    DbMessageQueue_push(db->queue, message);
    m_thread_join(db->thread);
    DbMessageQueue_clear(db->queue);

    free((char*)db->name);
    sqlite3_close(db->conn);
    path_free(db->path);
    free(db);
}
