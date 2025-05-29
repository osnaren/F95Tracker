#pragma once

#include <dcimgui/dcimgui.h>
#include <std.h>

#define LABEL_DFLT_COLOR "#696969"

typedef int32_t LabelId;

typedef struct {
    LabelId id;
    m_string_t name;
    ImColor color;
    int32_t position;
} Label;

void Label_init(Label* label);
void Label_init_set(Label* label, const Label* src);
void Label_set(Label* label, const Label* src);
void Label_clear(Label* label);
#define M_OPL_Label()                    \
    M_OPEXTEND(                          \
        M_POD_OPLIST,                    \
        INIT(API_2(Label_init)),         \
        INIT_SET(API_6(Label_init_set)), \
        SET(API_6(Label_set)),           \
        CLEAR(API_2(Label_clear)))

M_LIST_DUAL_PUSH_DEF(LabelList, Label)
#define M_OPL_LabelList_t() M_LIST_OPLIST(LabelList)

void label_list_update_positions(LabelList_t* labels);
