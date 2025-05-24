#pragma once

#include <dcimgui/dcimgui.h>
#include <fonts/mdi.h>
#include <std.h>

#define DEFAULT_TAB_ICON_STR mdi_heart_box

typedef int32_t TabId;

typedef struct {
    TabId id;
    m_string_t name;
    m_string_t icon;
    ImColor color;
    int32_t position;
} Tab;

static void Tab_init(Tab* tab) {
    tab->id = 0;
    m_string_init(tab->name);
    m_string_init(tab->icon);
    tab->color = (ImColor){{0, 0, 0, 0}};
    tab->position = 0;
}

static void Tab_init_set(Tab* tab, const Tab* src) {
    tab->id = src->id;
    m_string_init_set(tab->name, src->name);
    m_string_init_set(tab->icon, src->icon);
    tab->color = src->color;
    tab->position = src->position;
}

static void Tab_set(Tab* tab, const Tab* src) {
    tab->id = src->id;
    m_string_set(tab->name, src->name);
    m_string_set(tab->icon, src->icon);
    tab->color = src->color;
    tab->position = src->position;
}

static void Tab_clear(Tab* tab) {
    m_string_clear(tab->name);
    m_string_clear(tab->icon);
}

#define M_OPL_Tab()                 \
    (INIT(API_2(Tab_init)),         \
     SET(API_6(Tab_set)),           \
     INIT_SET(API_6(Tab_init_set)), \
     CLEAR(API_2(Tab_clear)),       \
     SWAP(M_SWAP_DEFAULT),          \
     EQUAL(API_6(M_EQUAL_DEFAULT)))

M_LIST_DUAL_PUSH_DEF(TabList, Tab)
#define M_OPL_TabList_t() M_LIST_OPLIST(TabList)
