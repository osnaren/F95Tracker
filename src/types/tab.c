#include "tab.h"
#include "db/db.h"

#include <app.h>

void Tab_init(Tab* tab) {
    tab->id = 0;
    m_string_init(tab->name);
    m_string_init(tab->icon);
    tab->color = (ImColor){{0, 0, 0, 0}};
    tab->position = 0;
}

void Tab_init_set(Tab* tab, const Tab* src) {
    tab->id = src->id;
    m_string_init_set(tab->name, src->name);
    m_string_init_set(tab->icon, src->icon);
    tab->color = src->color;
    tab->position = src->position;
}

void Tab_set(Tab* tab, const Tab* src) {
    tab->id = src->id;
    m_string_set(tab->name, src->name);
    m_string_set(tab->icon, src->icon);
    tab->color = src->color;
    tab->position = src->position;
}

void Tab_clear(Tab* tab) {
    m_string_clear(tab->name);
    m_string_clear(tab->icon);
}

void tab_list_update_positions(TabList_t* tabs) {
    int32_t i = -1;
    for
        M_EACH(tab, *tabs, TabList_t) {
            if(tab->position != ++i) {
                tab->position = i;
                db_save_tab(app.db, tab, TabsColumn_position);
            }
        }
}
