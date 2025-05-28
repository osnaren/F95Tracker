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
#define M_OPL_Label()                 \
    (INIT(API_2(Label_init)),         \
     SET(API_6(Label_set)),           \
     INIT_SET(API_6(Label_init_set)), \
     CLEAR(API_2(Label_clear)),       \
     SWAP(M_SWAP_DEFAULT),            \
     EQUAL(API_6(M_EQUAL_DEFAULT)))

M_LIST_DUAL_PUSH_DEF(LabelList, Label)
#define M_OPL_LabelList_t() M_LIST_OPLIST(LabelList)

void label_list_update_positions(LabelList_t* labels);
