#pragma once

#include <dcimgui/dcimgui.h>
#include <fonts/mdi.h>
#include <std.h>

#define TAB_DFLT_ICON mdi_heart_box

typedef int32_t TabId;

M_TUPLE_DEF2_AS(
    tab,
    Tab,
    (id, TabId),
    (name, m_string_t),
    (icon, m_string_t),
    (color, ImColor, M_POD_OPLIST),
    (position, int32_t))
#define M_OPL_Tab() M_A1_OPLIST
typedef tab_ptr Tab_ptr;

M_LIST_DUAL_PUSH_DEF_AS(tab_list, TabList, TabListIt, Tab)
#define M_OPL_TabList() M_LIST_OPLIST(tab_list)

void tab_list_update_positions(TabList* tabs);
