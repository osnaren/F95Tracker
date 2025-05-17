#pragma once

#include "settings/settings.h"

#include <std.h>

#define DB_COLUMN_ENUM(enum_name, column, ...) enum_name##_##column,
#define DB_EMPTY(...)
#define DB_TABLE_DECLARE(TABLE_MACRO, table_name, enum_name) \
    typedef enum {                                           \
        TABLE_MACRO(enum_name, DB_COLUMN_ENUM, DB_EMPTY)     \
    } enum_name;

#define _SETTINGS($, COLUMN, RENAME)                                                             \
    COLUMN($, _, .type = "INTEGER", .primary_key = true, .extra = "CHECK (_=0)")                 \
    COLUMN($, background_on_close, .type = "INTEGER", .dflt = "0")                               \
    COLUMN($, bg_notifs_interval, .type = "INTEGER", .dflt = "15")                               \
    COLUMN($, bg_refresh_interval, .type = "INTEGER", .dflt = "30")                              \
    COLUMN($, browser_custom_arguments, .type = "TEXT", .dflt = "''")                            \
    COLUMN($, browser_custom_executable, .type = "TEXT", .dflt = "''")                           \
    COLUMN($, browser_html, .type = "INTEGER", .dflt = "0")                                      \
    COLUMN($, browser_private, .type = "INTEGER", .dflt = "0")                                   \
    COLUMN($, browser, .type = "INTEGER", .dflt = "0") /* Integrated */                          \
    COLUMN($, cell_image_ratio, .type = "REAL", .dflt = "3.0")                                   \
    COLUMN($, check_notifs, .type = "INTEGER", .dflt = "0")                                      \
    COLUMN($, compact_timeline, .type = "INTEGER", .dflt = "0")                                  \
    COLUMN($, confirm_on_remove, .type = "INTEGER", .dflt = "1")                                 \
    COLUMN($, copy_urls_as_bbcode, .type = "INTEGER", .dflt = "0")                               \
    COLUMN($, datestamp_format, .type = "TEXT", .dflt = "'%b %d, %Y'")                           \
    COLUMN($, default_exe_dir, .type = "TEXT", .dflt = "'{}'")                                   \
    COLUMN($, default_tab_is_new, .type = "INTEGER", .dflt = "0")                                \
    COLUMN($, display_mode, .type = "INTEGER", .dflt = "1") /* DisplayMode_List */               \
    COLUMN($, display_tab, .type = "INTEGER", .dflt = "NULL")                                    \
    COLUMN($, downloads_dir, .type = "TEXT", .dflt = "'{}'")                                     \
    COLUMN($, ext_background_add, .type = "INTEGER", .dflt = "1")                                \
    COLUMN($, ext_highlight_tags, .type = "INTEGER", .dflt = "1")                                \
    COLUMN($, ext_icon_glow, .type = "INTEGER", .dflt = "1")                                     \
    COLUMN($, filter_all_tabs, .type = "INTEGER", .dflt = "0")                                   \
    COLUMN($, fit_images, .type = "INTEGER", .dflt = "0")                                        \
    COLUMN($, grid_columns, .type = "INTEGER", .dflt = "3")                                      \
    COLUMN($, hidden_timeline_events, .type = "TEXT", .dflt = "'[]'")                            \
    COLUMN($, hide_empty_tabs, .type = "INTEGER", .dflt = "0")                                   \
    COLUMN($, highlight_tags, .type = "INTEGER", .dflt = "1")                                    \
    COLUMN($, ignore_semaphore_timeouts, .type = "INTEGER", .dflt = "0")                         \
    COLUMN($, independent_tab_views, .type = "INTEGER", .dflt = "0")                             \
    COLUMN($, insecure_ssl, .type = "INTEGER", .dflt = "0")                                      \
    COLUMN($, interface_scaling, .type = "REAL", .dflt = "1.0")                                  \
    COLUMN($, last_successful_refresh, .type = "INTEGER", .dflt = "0")                           \
    COLUMN($, manual_sort_list, .type = "TEXT", .dflt = "'[]'")                                  \
    COLUMN($, mark_installed_after_add, .type = "INTEGER", .dflt = "0")                          \
    COLUMN($, max_connections, .type = "INTEGER", .dflt = "10")                                  \
    COLUMN($, max_retries, .type = "INTEGER", .dflt = "2")                                       \
    COLUMN($, notifs_show_update_banner, .type = "INTEGER", .dflt = "1")                         \
    COLUMN($, play_gifs, .type = "INTEGER", .dflt = "1")                                         \
    COLUMN($, play_gifs_unfocused, .type = "INTEGER", .dflt = "0")                               \
    COLUMN($, preload_nearby_images, .type = "INTEGER", .dflt = "0")                             \
    COLUMN($, proxy_type, .type = "INTEGER", .dflt = "1") /* ProxyType_Disabled */               \
    COLUMN($, proxy_host, .type = "TEXT", .dflt = "''")                                          \
    COLUMN($, proxy_port, .type = "INTEGER", .dflt = "8080")                                     \
    COLUMN($, proxy_username, .type = "TEXT", .dflt = "''")                                      \
    COLUMN($, proxy_password, .type = "TEXT", .dflt = "''")                                      \
    COLUMN($, quick_filters, .type = "INTEGER", .dflt = "1")                                     \
    COLUMN($, refresh_archived_games, .type = "INTEGER", .dflt = "1")                            \
    COLUMN($, refresh_completed_games, .type = "INTEGER", .dflt = "1")                           \
    COLUMN($, render_when_unfocused, .type = "INTEGER", .dflt = "1")                             \
    COLUMN($, request_timeout, .type = "INTEGER", .dflt = "30")                                  \
    COLUMN($, rpc_enabled, .type = "INTEGER", .dflt = "1")                                       \
    COLUMN($, rpdl_password, .type = "TEXT", .dflt = "''")                                       \
    COLUMN($, rpdl_token, .type = "TEXT", .dflt = "''")                                          \
    COLUMN($, rpdl_username, .type = "TEXT", .dflt = "''")                                       \
    COLUMN($, scroll_amount, .type = "REAL", .dflt = "1.0")                                      \
    COLUMN($, scroll_smooth, .type = "INTEGER", .dflt = "1")                                     \
    COLUMN($, scroll_smooth_speed, .type = "REAL", .dflt = "8.0")                                \
    COLUMN($, select_executable_after_add, .type = "INTEGER", .dflt = "0")                       \
    COLUMN($, show_remove_btn, .type = "INTEGER", .dflt = "0")                                   \
    COLUMN($, software_webview, .type = "INTEGER", .dflt = "0")                                  \
    COLUMN($, start_in_background, .type = "INTEGER", .dflt = "0")                               \
    COLUMN($, start_refresh, .type = "INTEGER", .dflt = "0")                                     \
    COLUMN($, style_accent, .type = "TEXT", .dflt = "'" DEFAULT_STYLE_ACCENT_HEX "'")            \
    COLUMN($, style_alt_bg, .type = "TEXT", .dflt = "'" DEFAULT_STYLE_ALT_BG_HEX "'")            \
    COLUMN($, style_bg, .type = "TEXT", .dflt = "'" DEFAULT_STYLE_BG_HEX "'")                    \
    COLUMN($, style_border, .type = "TEXT", .dflt = "'" DEFAULT_STYLE_BORDER_HEX "'")            \
    COLUMN($, style_corner_radius, .type = "INTEGER", .dflt = XSTR(DEFAULT_STYLE_CORNER_RADIUS)) \
    COLUMN($, style_text, .type = "TEXT", .dflt = "'" DEFAULT_STYLE_TEXT_HEX "'")                \
    COLUMN($, style_text_dim, .type = "TEXT", .dflt = "'" DEFAULT_STYLE_TEXT_DIM_HEX "'")        \
    COLUMN($, table_header_outside_list, .type = "INTEGER", .dflt = "1")                         \
    COLUMN($, tags_highlights, .type = "TEXT", .dflt = "'{}'")                                   \
    COLUMN($, tex_compress, .type = "INTEGER", .dflt = "1") /* TexCompress_Disabled */           \
    COLUMN($, tex_compress_replace, .type = "INTEGER", .dflt = "0")                              \
    COLUMN($, timestamp_format, .type = "TEXT", .dflt = "'%d/%m/%Y %H:%M'")                      \
    COLUMN($, unload_offscreen_images, .type = "INTEGER", .dflt = "0")                           \
    COLUMN($, vsync_ratio, .type = "INTEGER", .dflt = "1")                                       \
    COLUMN($, weighted_score, .type = "INTEGER", .dflt = "0")                                    \
    COLUMN($, zoom_area, .type = "INTEGER", .dflt = "50")                                        \
    COLUMN($, zoom_enabled, .type = "INTEGER", .dflt = "1")                                      \
    COLUMN($, zoom_times, .type = "REAL", .dflt = "4.0")                                         \
    RENAME($, grid_image_ratio, cell_image_ratio)                                                \
    RENAME($, minimize_on_close, background_on_close)                                            \
    RENAME($, start_in_tray, start_in_background)                                                \
    RENAME($, tray_notifs_interval, bg_notifs_interval)                                          \
    RENAME($, tray_refresh_interval, bg_refresh_interval)                                        \
    RENAME($, refresh_workers, max_connections)
DB_TABLE_DECLARE(_SETTINGS, settings, SettingsColumn)

typedef struct Db Db;

Db* db_init(void);
void db_load_settings(Db* db, Settings* settings);
void db_save_settings(Db* db, const Settings* settings, SettingsColumn column);
void db_free(Db* db);
