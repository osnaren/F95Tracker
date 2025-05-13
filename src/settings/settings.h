#pragma once

#include <dcimgui/dcimgui.h>
#include <smart_enum/smart_enum.h>
#include <std.h>

#define DisplayMode(_)    \
    _(DisplayModeList, 1) \
    _(DisplayModeGrid, 2) \
    _(DisplayModeKanban, 3)
SMARTENUM_DECLARE(DisplayMode, DisplayMode)
typedef struct {
    const char* icon;
} DisplayModeInfo;
extern const DisplayModeInfo display_mode[];

#define Os(_)       \
    _(OsWindows, 1) \
    _(OsLinux, 2)   \
    _(OsMacOS, 3)
SMARTENUM_DECLARE(Os, Os)

#define ProxyType(_)        \
    _(ProxyTypeDisabled, 1) \
    _(ProxyTypeSOCKS4, 2)   \
    _(ProxyTypeSOCKS5, 3)   \
    _(ProxyTypeHTTP, 4)
SMARTENUM_DECLARE(ProxyType, ProxyType)

#define TagHighlight(_)        \
    _(TagHighlightPositive, 1) \
    _(TagHighlightNegative, 2) \
    _(TagHighlightCritical, 3)
SMARTENUM_DECLARE(TagHighlight, TagHighlight)
typedef struct {
    ImColor color;
} TagHighlightInfo;
extern const TagHighlightInfo tag_highlight[];

#define TexCompress(_)        \
    _(TexCompressDisabled, 1) \
    _(TexCompressASTC, 2)     \
    _(TexCompressBC7, 3)
SMARTENUM_DECLARE(TexCompress, TexCompress)

#define TimelineEventType(_)                            \
    _(TimelineEventTypeGameAdded, 1)                    \
    _(TimelineEventTypeGameLaunched, 2)                 \
    _(TimelineEventTypeGameFinished, 3)                 \
    _(TimelineEventTypeGameInstalled, 4)                \
    _(TimelineEventTypeChangedName, 5)                  \
    _(TimelineEventTypeChangedStatus, 6)                \
    _(TimelineEventTypeChangedVersion, 7)               \
    _(TimelineEventTypeChangedDeveloper, 8)             \
    _(TimelineEventTypeChangedType, 9)                  \
    _(TimelineEventTypeTagsAdded, 10)                   \
    _(TimelineEventTypeTagsRemoved, 11)                 \
    _(TimelineEventTypeScoreIncreased, 12)              \
    _(TimelineEventTypeScoreDecreased, 13)              \
    _(TimelineEventTypeRecheckExpired, 14) /* Unused */ \
    _(TimelineEventTypeRecheckUserReq, 15)
SMARTENUM_DECLARE(TimelineEventType, TimelineEventType)
typedef struct {
    const char* display;
    const char* icon;
    uint8_t args_min;
    const char* template;
} TimelineEventTypeInfo;
extern const TimelineEventTypeInfo timeline_event_type[];

typedef struct {
    bool background_on_close;
    uint16_t bg_notifs_interval;
    uint16_t bg_refresh_interval;
    // Browser.get browser;
    // str browser_custom_arguments;
    // str browser_custom_executable;
    bool browser_html;
    bool browser_private;
    flt32_t cell_image_ratio;
    bool check_notifs;
    bool compact_timeline;
    bool confirm_on_remove;
    bool copy_urls_as_bbcode;
    // str datestamp_format;
    // dict[Os, str] default_exe_dir;
    bool default_tab_is_new;
    DisplayMode display_mode;
    // Tab.get display_tab;
    // dict[Os, str] downloads_dir;
    bool ext_background_add;
    bool ext_highlight_tags;
    bool ext_icon_glow;
    bool filter_all_tabs;
    bool fit_images;
    uint8_t grid_columns;
    // list[TimelineEventType] hidden_timeline_events;
    bool hide_empty_tabs;
    bool highlight_tags;
    bool ignore_semaphore_timeouts;
    bool independent_tab_views;
    bool insecure_ssl;
    float interface_scaling;
    // Timestamp last_successful_refresh;
    // list[int] manual_sort_list;
    bool mark_installed_after_add;
    uint8_t max_connections;
    uint8_t max_retries;
    bool notifs_show_update_banner;
    bool play_gifs;
    bool play_gifs_unfocused;
    bool preload_nearby_images;
    ProxyType proxy_type;
    // str proxy_host;
    uint16_t proxy_port;
    // str proxy_username;
    // str proxy_password;
    bool quick_filters;
    bool refresh_archived_games;
    bool refresh_completed_games;
    bool render_when_unfocused;
    uint8_t request_timeout;
    bool rpc_enabled;
    // str rpdl_password;
    // str rpdl_token;
    // str rpdl_username;
    flt32_t scroll_amount;
    bool scroll_smooth;
    flt32_t scroll_smooth_speed;
    bool select_executable_after_add;
    bool show_remove_btn;
    bool software_webview;
    bool start_in_background;
    bool start_refresh;
    // tuple[float] style_accent;
    // tuple[float] style_alt_bg;
    // tuple[float] style_bg;
    // tuple[float] style_border;
    uint8_t style_corner_radius;
    // tuple[float] style_text;
    // tuple[float] style_text_dim;
    bool table_header_outside_list;
    // dict[Tag, TagHighlight] tags_highlights;
    TexCompress tex_compress;
    bool tex_compress_replace;
    // str timestamp_format;
    bool unload_offscreen_images;
    uint8_t vsync_ratio;
    bool weighted_score;
    uint16_t zoom_area;
    bool zoom_enabled;
    flt32_t zoom_times;
} Settings;
