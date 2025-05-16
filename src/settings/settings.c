#include "settings.h"

#include <fonts/mdi.h>

SMARTENUM_DEFINE(_DisplayMode, DisplayMode)
const DisplayModeInfo display_mode[] = {
    [DisplayMode_List] = {.icon = mdi_view_agenda_outline},
    [DisplayMode_Grid] = {.icon = mdi_view_grid_outline},
    [DisplayMode_Kanban] = {.icon = mdi_view_week_outline},
};

SMARTENUM_DEFINE(_Os, Os)

SMARTENUM_DEFINE(_ProxyType, ProxyType)

SMARTENUM_DEFINE(_TagHighlight, TagHighlight)
const TagHighlightInfo tag_highlight[] = {
    [TagHighlight_Positive] = {.color = {{0.0, 0.6, 0.0, 1.0}}},
    [TagHighlight_Negative] = {.color = {{0.6, 0.0, 0.0, 1.0}}},
    [TagHighlight_Critical] = {.color = {{0.0, 0.0, 0.0, 1.0}}},
};

SMARTENUM_DEFINE(_TexCompress, TexCompress)

SMARTENUM_DEFINE(_TimelineEventType, TimelineEventType)
const TimelineEventTypeInfo timeline_event_type[] = {
    [TimelineEventType_GameAdded] =
        {
            .display = "Added",
            .icon = mdi_alert_decagram,
            .args_min = 0,
            .template = "Added to the library",
        },
    [TimelineEventType_GameLaunched] =
        {
            .display = "Launched",
            .icon = mdi_play,
            .args_min = 1,
            .template = "Launched {}",
        },
    [TimelineEventType_GameFinished] =
        {
            .display = "Finished",
            .icon = mdi_flag_checkered,
            .args_min = 1,
            .template = "Finished {}",
        },
    [TimelineEventType_GameInstalled] =
        {
            .display = "Installed",
            .icon = mdi_download,
            .args_min = 1,
            .template = "Installed {}",
        },
    [TimelineEventType_ChangedName] =
        {
            .display = "Changed name",
            .icon = mdi_spellcheck,
            .args_min = 2,
            .template = "Name changed from \"{}\" to \"{}\"",
        },
    [TimelineEventType_ChangedStatus] =
        {
            .display = "Changed status",
            .icon = mdi_lightning_bolt,
            .args_min = 2,
            .template = "Status changed from \"{}\" to \"{}\"",
        },
    [TimelineEventType_ChangedVersion] =
        {
            .display = "Changed version",
            .icon = mdi_star,
            .args_min = 2,
            .template = "Version changed from \"{}\" to \"{}\"",
        },
    [TimelineEventType_ChangedDeveloper] =
        {
            .display = "Changed developer",
            .icon = mdi_account,
            .args_min = 2,
            .template = "Developer changed from \"{}\" to \"{}\"",
        },
    [TimelineEventType_ChangedType] =
        {
            .display = "Changed type",
            .icon = mdi_shape,
            .args_min = 2,
            .template = "Type changed from \"{}\" to \"{}\"",
        },
    [TimelineEventType_TagsAdded] =
        {
            .display = "Tags added",
            .icon = mdi_tag_plus,
            .args_min = 1,
            .template = "Tags were added: {}",
        },
    [TimelineEventType_TagsRemoved] =
        {
            .display = "Tags removed",
            .icon = mdi_tag_minus,
            .args_min = 1,
            .template = "Tags were removed: {}",
        },
    [TimelineEventType_ScoreIncreased] =
        {
            .display = "Score increased",
            .icon = mdi_thumb_up,
            .args_min = 4,
            .template = "Forum score increased from {} ({}) to {} ({})",
        },
    [TimelineEventType_ScoreDecreased] =
        {
            .display = "Score decreased",
            .icon = mdi_thumb_down,
            .args_min = 4,
            .template = "Forum score decreased from {} ({}) to {} ({})",
        },
    [TimelineEventType_RecheckExpired] =
        {
            .display = "Recheck expired",
            .icon = mdi_timer_sync,
            .args_min = 1,
            .template =
                "Forcefully performed a full recheck because game has remained idle for {} day(s)",
        },
    [TimelineEventType_RecheckUserReq] =
        {
            .display = "Recheck requested",
            .icon = mdi_reload_alert,
            .args_min = 0,
            .template = "Forcefully performed a full recheck requested by user",
        },
};

Settings* settings_init(void) {
    Settings* settings = malloc(sizeof(Settings));

    m_string_init(settings->browser_custom_arguments);
    m_string_init(settings->browser_custom_executable);
    m_string_init(settings->datestamp_format);
    m_string_init(settings->proxy_host);
    m_string_init(settings->proxy_username);
    m_string_init(settings->proxy_password);
    m_string_init(settings->rpdl_password);
    m_string_init(settings->rpdl_token);
    m_string_init(settings->rpdl_username);
    m_string_init(settings->timestamp_format);

    return settings;
}

void settings_free(Settings* settings) {
    m_string_clear(settings->browser_custom_arguments);
    m_string_clear(settings->browser_custom_executable);
    m_string_clear(settings->datestamp_format);
    m_string_clear(settings->proxy_host);
    m_string_clear(settings->proxy_username);
    m_string_clear(settings->proxy_password);
    m_string_clear(settings->rpdl_password);
    m_string_clear(settings->rpdl_token);
    m_string_clear(settings->rpdl_username);
    m_string_clear(settings->timestamp_format);

    free(settings);
}
