#include "settings.h"

#include <fonts/mdi.h>

SMARTENUM_DEFINE(_DisplayMode, DisplayMode)
const DisplayModeInfo display_mode[] = {
    [DisplayModeList] = {.icon = mdi_view_agenda_outline},
    [DisplayModeGrid] = {.icon = mdi_view_grid_outline},
    [DisplayModeKanban] = {.icon = mdi_view_week_outline},
};

SMARTENUM_DEFINE(_Os, Os)

SMARTENUM_DEFINE(_ProxyType, ProxyType)

SMARTENUM_DEFINE(_TagHighlight, TagHighlight)
const TagHighlightInfo tag_highlight[] = {
    [TagHighlightPositive] = {.color = {{0.0, 0.6, 0.0, 1.0}}},
    [TagHighlightNegative] = {.color = {{0.6, 0.0, 0.0, 1.0}}},
    [TagHighlightCritical] = {.color = {{0.0, 0.0, 0.0, 1.0}}},
};

SMARTENUM_DEFINE(_TexCompress, TexCompress)

SMARTENUM_DEFINE(_TimelineEventType, TimelineEventType)
const TimelineEventTypeInfo timeline_event_type[] = {
    [TimelineEventTypeGameAdded] =
        {
            .display = "Added",
            .icon = "alert_decagram",
            .args_min = 0,
            .template = "Added to the library",
        },
    [TimelineEventTypeGameLaunched] =
        {
            .display = "Launched",
            .icon = "play",
            .args_min = 1,
            .template = "Launched {}",
        },
    [TimelineEventTypeGameFinished] =
        {
            .display = "Finished",
            .icon = "flag_checkered",
            .args_min = 1,
            .template = "Finished {}",
        },
    [TimelineEventTypeGameInstalled] =
        {
            .display = "Installed",
            .icon = "download",
            .args_min = 1,
            .template = "Installed {}",
        },
    [TimelineEventTypeChangedName] =
        {
            .display = "Changed name",
            .icon = "spellcheck",
            .args_min = 2,
            .template = "Name changed from \"{}\" to \"{}\"",
        },
    [TimelineEventTypeChangedStatus] =
        {
            .display = "Changed status",
            .icon = "lightning_bolt",
            .args_min = 2,
            .template = "Status changed from \"{}\" to \"{}\"",
        },
    [TimelineEventTypeChangedVersion] =
        {
            .display = "Changed version",
            .icon = "star",
            .args_min = 2,
            .template = "Version changed from \"{}\" to \"{}\"",
        },
    [TimelineEventTypeChangedDeveloper] =
        {
            .display = "Changed developer",
            .icon = "account",
            .args_min = 2,
            .template = "Developer changed from \"{}\" to \"{}\"",
        },
    [TimelineEventTypeChangedType] =
        {
            .display = "Changed type",
            .icon = "shape",
            .args_min = 2,
            .template = "Type changed from \"{}\" to \"{}\"",
        },
    [TimelineEventTypeTagsAdded] =
        {
            .display = "Tags added",
            .icon = "tag_plus",
            .args_min = 1,
            .template = "Tags were added: {}",
        },
    [TimelineEventTypeTagsRemoved] =
        {
            .display = "Tags removed",
            .icon = "tag_minus",
            .args_min = 1,
            .template = "Tags were removed: {}",
        },
    [TimelineEventTypeScoreIncreased] =
        {
            .display = "Score increased",
            .icon = "thumb_up",
            .args_min = 4,
            .template = "Forum score increased from {} ({}) to {} ({})",
        },
    [TimelineEventTypeScoreDecreased] =
        {
            .display = "Score decreased",
            .icon = "thumb_down",
            .args_min = 4,
            .template = "Forum score decreased from {} ({}) to {} ({})",
        },
    [TimelineEventTypeRecheckExpired] =
        {
            .display = "Recheck expired",
            .icon = "timer_sync",
            .args_min = 1,
            .template =
                "Forcefully performed a full recheck because game has remained idle for {} day(s)",
        },
    [TimelineEventTypeRecheckUserReq] =
        {
            .display = "Recheck requested",
            .icon = "reload_alert",
            .args_min = 0,
            .template = "Forcefully performed a full recheck requested by user",
        },
};

// def __post_init__(self):
//     if "" in self.default_exe_dir:
//     from modules import globals
//     self.default_exe_dir[globals.os] = self.default_exe_dir[""]
//     del self.default_exe_dir[""]
