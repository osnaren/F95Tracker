#include "settings.h"

Settings* settings_init(void) {
    Settings* settings = malloc(sizeof(Settings));

    m_string_init(settings->browser_custom_arguments);
    m_string_init(settings->browser_custom_executable);
    m_string_init(settings->datestamp_format);
    for(Os os = Os_min(); os <= Os_max(); os++) {
        settings->default_exe_dir[os] = path_init("");
    }
    for(Os os = Os_min(); os <= Os_max(); os++) {
        settings->downloads_dir[os] = path_init("");
    }
    game_id_array_init(settings->manual_sort_list);
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
    for(Os os = Os_min(); os <= Os_max(); os++) {
        path_free(settings->default_exe_dir[os]);
    }
    for(Os os = Os_min(); os <= Os_max(); os++) {
        path_free(settings->downloads_dir[os]);
    }
    game_id_array_clear(settings->manual_sort_list);
    m_string_clear(settings->proxy_host);
    m_string_clear(settings->proxy_username);
    m_string_clear(settings->proxy_password);
    m_string_clear(settings->rpdl_password);
    m_string_clear(settings->rpdl_token);
    m_string_clear(settings->rpdl_username);
    m_string_clear(settings->timestamp_format);

    free(settings);
}
