#pragma once

#include <dcimgui/dcimgui.h>
#include <std.h>

#define LABEL_DFLT_COLOR "#696969"

typedef int32_t LabelId;

M_TUPLE_DEF2_AS(
    label,
    Label,
    (id, LabelId),
    (name, m_string_t),
    (color, ImColor, M_POD_OPLIST),
    (position, int32_t))
#define M_OPL_Label() M_A1_OPLIST
typedef label_ptr Label_ptr;

M_LIST_DUAL_PUSH_DEF_AS(label_list, LabelList, LabelListIt, Label)
#define M_OPL_LabelList() M_LIST_OPLIST(label_list)
typedef label_list_ptr LabelList_ptr;

void label_list_update_positions(LabelList_ptr labels);

M_LIST_DUAL_PUSH_DEF_AS(label_ptr_list, LabelPtrList, LabelPtrListIt, Label_ptr, M_PTR_OPLIST)
#define M_OPL_LabelPtrList() M_LIST_OPLIST(label_ptr_list)
