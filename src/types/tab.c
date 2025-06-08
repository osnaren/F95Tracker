#include "tab.h"
#include "db/db.h"

#include <app.h>

void tab_list_update_positions(TabList* tabs) {
    int32_t i = -1;
    for each(Tab_ptr, tab, TabList, *tabs) {
        if(tab->position != ++i) {
            tab->position = i;
            db_save_tab(app.db, tab, TabsColumn_position);
        }
    }
}
