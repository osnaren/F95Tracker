#pragma once

#include <std.h>

typedef struct Path Path;

Path* path_init(const char* init);
Path* path_init_data_dir(void);

Path* path_dup(Path* path);

const char* path_cstr(Path* path);
const char* path_name(Path* path);
const char* path_extension(Path* path);

void path_set(Path* path, const char* set);
void path_set_name(Path* path, const char* name);
void path_set_extension(Path* path, const char* extension);

void path_normalize(Path* path);
void path_parent(Path* path);
void path_join(Path* path, const char* join);

bool path_is_empty(Path* path);
bool path_is_file(Path* path);
bool path_is_dir(Path* path);

bool path_mkdir(Path* path, bool recursive);
bool path_read_lines(Path* path, m_string_list_ptr lines);
bool path_read_text(Path* path, m_string_ptr string);
bool path_read_bytes(Path* path, m_bstring_ptr bytes);
bool path_write_raw(Path* path, const void* data, size_t size);
bool path_write_text(Path* path, m_string_ptr string);
bool path_write_bytes(Path* path, m_bstring_ptr bytes);

void path_free(Path* path);
