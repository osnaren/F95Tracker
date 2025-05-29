#include "label.h"
#include "db/db.h"

#include <app.h>

void Label_init(Label* label) {
    m_string_init(label->name);
    label->color = (ImColor){{0, 0, 0, 0}};
}

void Label_init_set(Label* label, const Label* src) {
    label->id = src->id;
    m_string_init_set(label->name, src->name);
    label->color = src->color;
    label->position = src->position;
}

void Label_set(Label* label, const Label* src) {
    label->id = src->id;
    m_string_set(label->name, src->name);
    label->color = src->color;
    label->position = src->position;
}

void Label_clear(Label* label) {
    m_string_clear(label->name);
}

void label_list_update_positions(LabelList_t* labels) {
    int32_t i = -1;
    for
        M_EACH(label, *labels, LabelList_t) {
            if(label->position != ++i) {
                label->position = i;
                db_save_label(app.db, label, LabelsColumn_position);
            }
        }
}
