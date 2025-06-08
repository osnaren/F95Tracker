#include "label.h"
#include "db/db.h"

#include <app.h>

void label_list_update_positions(LabelList* labels) {
    int32_t i = -1;
    for each(Label_ptr, label, LabelList, *labels) {
        if(label->position != ++i) {
            label->position = i;
            db_save_label(app.db, label, LabelsColumn_position);
        }
    }
}
