#pragma once

#include <dcimgui/dcimgui.h>
#include <fonts/mdi.h>
#include <std.h>

#define TAB_DFLT_ICON mdi_heart_box

typedef int32_t TabId;

typedef struct {
    TabId id;
    m_string_t name;
    m_string_t icon;
    ImColor color;
    int32_t position;
} Tab;

void Tab_init(Tab* tab);
void Tab_init_set(Tab* tab, const Tab* src);
void Tab_set(Tab* tab, const Tab* src);
void Tab_clear(Tab* tab);
#define M_OPL_Tab()                    \
    M_OPEXTEND(                        \
        M_POD_OPLIST,                  \
        INIT(API_2(Tab_init)),         \
        INIT_SET(API_6(Tab_init_set)), \
        SET(API_6(Tab_set)),           \
        CLEAR(API_2(Tab_clear)))

M_LIST_DUAL_PUSH_DEF_AS(tab_list, TabList, TabListIt, Tab)
#define M_OPL_TabList() M_LIST_OPLIST(tab_list)

void tab_list_update_positions(TabList* tabs);
