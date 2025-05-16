#include "db.h"
#include "path/path.h"

#include <sqlite3/sqlite3.h>

#define sqlite3_column_text(pStmt, i) (const char*)sqlite3_column_text(pStmt, i)
#define sqlite3_column_count(pStmt)   (size_t)sqlite3_column_count(pStmt)

struct Db {
    sqlite3* conn;
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

#define DB_TABLE(table_name, columns_array, renames_array)           \
    static const DbColumn _##table_name##_columns[] = columns_array; \
    static const DbRename _##table_name##_renames[] = renames_array; \
    static const DbTable table_name##_table = {                      \
        .name = #table_name,                                         \
        .columns = _##table_name##_columns,                          \
        .columns_count = COUNT_OF(_##table_name##_columns),          \
        .renames = _##table_name##_renames,                          \
        .renames_count = COUNT_OF(_##table_name##_renames),          \
    }
#define DB_COLUMNS(...) __VA_ARGS__
#define DB_RENAMES(...) __VA_ARGS__

static void db_perror(Db* db, const char* s) {
    custom_perror(s, sqlite3_errmsg(db->conn));
}

#define db_assert(db, res, exp, s) \
    if(res != exp) {               \
        db_perror(db, s);          \
    }                              \
    assert(res == exp)

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

static void db_create_table(Db* db, const DbTable* table) {
    m_string_t sql;
    m_string_init_printf(sql, "CREATE TABLE IF NOT EXISTS %s (", table->name);
    for(size_t col = 0; col < table->columns_count; col++) {
        const DbColumn* column = &table->columns[col];
        db_append_column_spec(&sql, column);
        m_string_cat(sql, ",");
    }
    m_string_strim(sql, ",");
    m_string_cat(sql, ")");

    int32_t res = sqlite3_exec(db->conn, m_string_get_cstr(sql), NULL, NULL, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_exec()");

    // Rename columns
    for(size_t ren = 0; ren < table->renames_count; ren++) {
        const DbRename* rename = &table->renames[ren];
        res = sqlite3_table_column_metadata(
            db->conn,
            NULL,
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
            NULL,
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
    sqlite3_stmt* stmt;
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

    // TODO: update column defs eg change type, default, etc...

    m_string_clear(sql);
}

DB_TABLE(
    settings,
    DB_COLUMNS({
        {.name = "_", .type = "INTEGER", .primary_key = true, .extra = "CHECK (_=0)"},
        {.name = "background_on_close", .type = "INTEGER", .dflt = "0"},
        {.name = "bg_notifs_interval", .type = "INTEGER", .dflt = "15"},
        {.name = "bg_refresh_interval", .type = "INTEGER", .dflt = "30"},
        {.name = "browser_custom_arguments", .type = "TEXT", .dflt = "''"},
        {.name = "browser_custom_executable", .type = "TEXT", .dflt = "''"},
        {.name = "browser_html", .type = "INTEGER", .dflt = "0"},
        {.name = "browser_private", .type = "INTEGER", .dflt = "0"},
        {.name = "browser", .type = "INTEGER", .dflt = "0"}, // Integrated
        {.name = "cell_image_ratio", .type = "REAL", .dflt = "3.0"},
        {.name = "check_notifs", .type = "INTEGER", .dflt = "0"},
        {.name = "compact_timeline", .type = "INTEGER", .dflt = "0"},
        {.name = "confirm_on_remove", .type = "INTEGER", .dflt = "1"},
        {.name = "copy_urls_as_bbcode", .type = "INTEGER", .dflt = "0"},
        {.name = "datestamp_format", .type = "TEXT", .dflt = "'%b %d, %Y'"},
        {.name = "default_exe_dir", .type = "TEXT", .dflt = "'{}'"},
        {.name = "default_tab_is_new", .type = "INTEGER", .dflt = "0"},
        {.name = "display_mode", .type = "INTEGER", .dflt = "1"}, // DisplayMode_List
        {.name = "display_tab", .type = "INTEGER", .dflt = "NULL"},
        {.name = "downloads_dir", .type = "TEXT", .dflt = "'{}'"},
        {.name = "ext_background_add", .type = "INTEGER", .dflt = "1"},
        {.name = "ext_highlight_tags", .type = "INTEGER", .dflt = "1"},
        {.name = "ext_icon_glow", .type = "INTEGER", .dflt = "1"},
        {.name = "filter_all_tabs", .type = "INTEGER", .dflt = "0"},
        {.name = "fit_images", .type = "INTEGER", .dflt = "0"},
        {.name = "grid_columns", .type = "INTEGER", .dflt = "3"},
        {.name = "hidden_timeline_events", .type = "TEXT", .dflt = "'[]'"},
        {.name = "hide_empty_tabs", .type = "INTEGER", .dflt = "0"},
        {.name = "highlight_tags", .type = "INTEGER", .dflt = "1"},
        {.name = "ignore_semaphore_timeouts", .type = "INTEGER", .dflt = "0"},
        {.name = "independent_tab_views", .type = "INTEGER", .dflt = "0"},
        {.name = "insecure_ssl", .type = "INTEGER", .dflt = "0"},
        {.name = "interface_scaling", .type = "REAL", .dflt = "1.0"},
        {.name = "last_successful_refresh", .type = "INTEGER", .dflt = "0"},
        {.name = "manual_sort_list", .type = "TEXT", .dflt = "'[]'"},
        {.name = "mark_installed_after_add", .type = "INTEGER", .dflt = "0"},
        {.name = "max_connections", .type = "INTEGER", .dflt = "10"},
        {.name = "max_retries", .type = "INTEGER", .dflt = "2"},
        {.name = "notifs_show_update_banner", .type = "INTEGER", .dflt = "1"},
        {.name = "play_gifs", .type = "INTEGER", .dflt = "1"},
        {.name = "play_gifs_unfocused", .type = "INTEGER", .dflt = "0"},
        {.name = "preload_nearby_images", .type = "INTEGER", .dflt = "0"},
        {.name = "proxy_type", .type = "INTEGER", .dflt = "1"}, // ProxyType_Disabled
        {.name = "proxy_host", .type = "TEXT", .dflt = "''"},
        {.name = "proxy_port", .type = "INTEGER", .dflt = "8080"},
        {.name = "proxy_username", .type = "TEXT", .dflt = "''"},
        {.name = "proxy_password", .type = "TEXT", .dflt = "''"},
        {.name = "quick_filters", .type = "INTEGER", .dflt = "1"},
        {.name = "refresh_archived_games", .type = "INTEGER", .dflt = "1"},
        {.name = "refresh_completed_games", .type = "INTEGER", .dflt = "1"},
        {.name = "render_when_unfocused", .type = "INTEGER", .dflt = "1"},
        {.name = "request_timeout", .type = "INTEGER", .dflt = "30"},
        {.name = "rpc_enabled", .type = "INTEGER", .dflt = "1"},
        {.name = "rpdl_password", .type = "TEXT", .dflt = "''"},
        {.name = "rpdl_token", .type = "TEXT", .dflt = "''"},
        {.name = "rpdl_username", .type = "TEXT", .dflt = "''"},
        {.name = "scroll_amount", .type = "REAL", .dflt = "1.0"},
        {.name = "scroll_smooth", .type = "INTEGER", .dflt = "1"},
        {.name = "scroll_smooth_speed", .type = "REAL", .dflt = "8.0"},
        {.name = "select_executable_after_add", .type = "INTEGER", .dflt = "0"},
        {.name = "show_remove_btn", .type = "INTEGER", .dflt = "0"},
        {.name = "software_webview", .type = "INTEGER", .dflt = "0"},
        {.name = "start_in_background", .type = "INTEGER", .dflt = "0"},
        {.name = "start_refresh", .type = "INTEGER", .dflt = "0"},
        {.name = "style_accent", .type = "TEXT", .dflt = "'{accent}'"}, // FIXME
        {.name = "style_alt_bg", .type = "TEXT", .dflt = "'{alt_bg}'"}, // FIXME
        {.name = "style_bg", .type = "TEXT", .dflt = "'{bg}'"}, // FIXME
        {.name = "style_border", .type = "TEXT", .dflt = "'{border}'"}, // FIXME
        {.name = "style_corner_radius", .type = "INTEGER", .dflt = "6"}, // FIXME
        {.name = "style_text", .type = "TEXT", .dflt = "'{text}'"}, // FIXME
        {.name = "style_text_dim", .type = "TEXT", .dflt = "'{text_dim}'"}, // FIXME
        {.name = "table_header_outside_list", .type = "INTEGER", .dflt = "1"},
        {.name = "tags_highlights", .type = "TEXT", .dflt = "'{}'"},
        {.name = "tex_compress", .type = "INTEGER", .dflt = "1"}, // TexCompress_Disabled
        {.name = "tex_compress_replace", .type = "INTEGER", .dflt = "0"},
        {.name = "timestamp_format", .type = "TEXT", .dflt = "'%d/%m/%Y %H:%M'"},
        {.name = "unload_offscreen_images", .type = "INTEGER", .dflt = "0"},
        {.name = "vsync_ratio", .type = "INTEGER", .dflt = "1"},
        {.name = "weighted_score", .type = "INTEGER", .dflt = "0"},
        {.name = "zoom_area", .type = "INTEGER", .dflt = "50"},
        {.name = "zoom_enabled", .type = "INTEGER", .dflt = "1"},
        {.name = "zoom_times", .type = "REAL", .dflt = "4.0"},
    }),
    DB_RENAMES({
        {.old = "grid_image_ratio", .new = "cell_image_ratio"},
        {.old = "minimize_on_close", .new = "background_on_close"},
        {.old = "start_in_tray", .new = "start_in_background"},
        {.old = "tray_notifs_interval", .new = "bg_notifs_interval"},
        {.old = "tray_refresh_interval", .new = "bg_refresh_interval"},
        {.old = "refresh_workers", .new = "max_connections"},
    }));

void db_load_settings(Db* db, Settings* settings) {
    db_create_table(db, &settings_table);

    m_string_t sql;
    m_string_init_set(sql, "SELECT ");
    for(size_t col = 0; col < settings_table.columns_count; col++) {
        m_string_cat(sql, settings_table.columns[col].name);
        m_string_cat(sql, ",");
    }
    m_string_strim(sql, ",");
    m_string_cat_printf(sql, " FROM %s", settings_table.name);

    sqlite3_stmt* stmt;
    int32_t res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
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
    // settings->grid_columns = sqlite3_column_int(stmt, column_i++);
    col++;
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

void db_free(Db* db) {
    sqlite3_close(db->conn);
    free(db);
}
