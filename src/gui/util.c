#include "util.h"

typedef struct {
    char** buf;
    int32_t* len;
} ImguiInputTextResizeCallbackCtx;

int32_t gui_util_input_text_resize_callback(ImGuiInputTextCallbackData* data) {
    ImguiInputTextResizeCallbackCtx* ctx = data->UserData;
    if(data->EventFlag & ImGuiInputTextFlags_CallbackResize && data->BufTextLen + 1 != *ctx->len) {
        *ctx->len = data->BufTextLen + 1;
        *ctx->buf = realloc(*ctx->buf, *ctx->len);
        data->Buf = *ctx->buf;
    }
    return 0;
}

bool gui_util_is_topmost() {
    return !ImGui_IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);
}

bool gui_util_should_close_weak_modal() {
    if(!gui_util_is_topmost()) {
        return false;
    }
    if(ImGui_Shortcut(ImGuiKey_Escape, ImGuiInputFlags_None)) {
        return true;
    }
    if(ImGui_IsMouseClicked(ImGuiMouseButton_Left)) {
        ImVec2 window_pos = ImGui_GetWindowPos();
        ImVec2 window_size = ImGui_GetWindowSize();
        if(!ImGui_IsMouseHoveringRectEx(
               window_pos,
               (ImVec2){window_pos.x + window_size.x, window_pos.y + window_size.y},
               false)) {
            return true;
        }
    }
    return false;
}
