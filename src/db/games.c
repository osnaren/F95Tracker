#include "games.h"
#include "db_i.h"

#include <app.h>

DB_TABLE_DEFINE(_GAMES, games, GamesColumn)

static void db_parse_game(Db* db, sqlite3_stmt* stmt, Game* game) {
    UNUSED(db);
    size_t col = 0;

    // FIXME: load missing fields
    game->id = sqlite3_column_int(stmt, col++);

    bool migrate_custom = sqlite3_column_type(stmt, col) == SQLITE_NULL;
    if(!migrate_custom) {
        game->custom = sqlite3_column_int(stmt, col);
    }
    col++;

    m_string_set(game->name, sqlite3_column_text(stmt, col++));
    m_string_set(game->version, sqlite3_column_text(stmt, col++));
    m_string_set(game->developer, sqlite3_column_text(stmt, col++));
    game->type = sqlite3_column_int(stmt, col++);
    game->status = sqlite3_column_int(stmt, col++);
    m_string_set(game->url, sqlite3_column_text(stmt, col++));
    game->added_on = sqlite3_column_int64(stmt, col++);
    game->last_updated = sqlite3_column_int64(stmt, col++);
    game->last_full_check = sqlite3_column_int64(stmt, col++);
    m_string_set(game->last_check_version, sqlite3_column_text(stmt, col++));
    game->last_launched = sqlite3_column_int64(stmt, col++);
    game->score = sqlite3_column_int(stmt, col++);
    game->votes = sqlite3_column_int(stmt, col++);
    game->rating = sqlite3_column_int(stmt, col++);
    m_string_set(game->finished, sqlite3_column_text(stmt, col++));
    m_string_set(game->installed, sqlite3_column_text(stmt, col++));

    bool migrate_updated = sqlite3_column_type(stmt, col) == SQLITE_NULL;
    if(!migrate_updated) {
        game->updated = sqlite3_column_int(stmt, col);
    }
    col++;

    game->archived = sqlite3_column_int(stmt, col++);

    const char* executables_text = sqlite3_column_text(stmt, col++);
    if(executables_text[0] == '[') {
        json_object* executables_json = json_tokener_parse(executables_text);
        for(size_t i = 0; i < json_object_array_length(executables_json); i++) {
            json_object* executable = json_object_array_get_idx(executables_json, i);
            m_string_t executable_str;
            m_string_init_set(executable_str, json_object_get_string(executable));
            MstringList_push_front_move(game->executables, &executable_str);
        }
        json_object_put(executables_json);
    } else if(executables_text[0] != '\0') {
        printf("%s\n", executables_text);
        m_string_t executable_str;
        m_string_init_set(executable_str, executables_text);
        MstringList_push_front_move(game->executables, &executable_str);
    }

    m_string_set(game->description, sqlite3_column_text(stmt, col++));
    m_string_set(game->changelog, sqlite3_column_text(stmt, col++));

    json_object* tags_json = sqlite3_column_json(stmt, col++);
    for(size_t i = 0; i < json_object_array_length(tags_json); i++) {
        json_object* tag = json_object_array_get_idx(tags_json, i);
        m_bitset_set_at(game->tags, json_object_get_int(tag), true);
    }
    json_object_put(tags_json);

    json_object* unknown_tags_json = sqlite3_column_json(stmt, col++);
    for(size_t i = 0; i < json_object_array_length(unknown_tags_json); i++) {
        json_object* unknown_tag = json_object_array_get_idx(unknown_tags_json, i);
        m_string_t unknown_tag_str;
        m_string_init_set(unknown_tag_str, json_object_get_string(unknown_tag));
        MstringList_push_front_move(game->unknown_tags, &unknown_tag_str);
    }
    json_object_put(unknown_tags_json);

    game->unknown_tags_flag = sqlite3_column_int(stmt, col++);
    // game->labels = sqlite3_column_int(stmt, col++);
    col++;

    game->tab = NULL;
    if(sqlite3_column_type(stmt, col) != SQLITE_NULL) {
        TabId tab_id = sqlite3_column_int(stmt, col);
        for
            M_EACH(tab, app.tabs, TabList_t) {
                if(tab->id == tab_id) {
                    game->tab = tab;
                    break;
                }
            }
    }
    col++;

    m_string_set(game->notes, sqlite3_column_text(stmt, col++));

    m_string_set(game->image_url, sqlite3_column_text(stmt, col++));
    if(m_string_equal_p(game->image_url, "-")) {
        m_string_set(game->image_url, GAME_IMAGE_URL_MISSING);
    }

    json_object* previews_urls_json = sqlite3_column_json(stmt, col++);
    for(size_t i = 0; i < json_object_array_length(previews_urls_json); i++) {
        json_object* preview_url = json_object_array_get_idx(previews_urls_json, i);
        m_string_t preview_url_str;
        m_string_init_set(preview_url_str, json_object_get_string(preview_url));
        MstringList_push_front_move(game->previews_urls, &preview_url_str);
    }
    json_object_put(previews_urls_json);

    // game->downloads = sqlite3_column_int(stmt, col++);
    col++;
    game->reviews_total = sqlite3_column_int(stmt, col++);
    // game->reviews = sqlite3_column_int(stmt, col++);
    col++;

    if(migrate_custom) {
        game->custom = game->status == GameStatus_Custom;
    }
    if(game->id < 0) {
        game->custom = true;
    }

    if(migrate_updated) {
        game->updated = !m_string_empty_p(game->installed) &&
                        !m_string_equal_p(game->installed, game->version);
    }

    if(m_string_equal_p(game->finished, "1") &&
       (!m_string_equal_p(game->installed, "1") && !m_string_equal_p(game->version, "1"))) {
        if(!m_string_empty_p(game->installed)) {
            m_string_set(game->finished, game->installed);
        } else {
            m_string_set(game->finished, game->version);
        }
    }

    if(m_string_equal_p(game->finished, "0") &&
       (!m_string_equal_p(game->installed, "0") && !m_string_equal_p(game->version, "0"))) {
        m_string_reset(game->finished);
    }
}

void db_do_load_games(Db* db, GameDict_t* games) {
    int32_t res;
    m_string_t sql;
    m_string_init(sql);

    // Create the table and handle schema migrations
    db_create_table(db, &games_table);

    // Read all games
    m_string_set(sql, "SELECT ");
    db_append_column_names(&sql, &games_table);
    m_string_cat_printf(sql, " FROM %s", games_table.name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    assert(sqlite3_column_count(stmt) == games_table.columns_count);
    while((res = sqlite3_step(stmt)) != SQLITE_DONE) {
        db_assert(db, res, SQLITE_ROW, "sqlite3_step()");
        Game* game = game_init();
        db_parse_game(db, stmt, game);
        GameDict_set_at(*games, game->id, game);
    }

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
}

void db_do_save_game(Db* db, const Game* game, GamesColumn column) {
    int32_t res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(
        sql,
        "UPDATE %s SET %s=? WHERE id=?",
        games_table.name,
        games_table.columns[column].name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    res = sqlite3_bind_int(stmt, 2, game->id);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    // FIXME: save missing fields
    switch(column) {
    case GamesColumn_id:
        res = sqlite3_bind_int(stmt, 1, game->id);
        break;
    case GamesColumn_custom:
        res = sqlite3_bind_int(stmt, 1, game->custom);
        break;
    case GamesColumn_name:
        res = sqlite3_bind_mstring(stmt, 1, game->name);
        break;
    case GamesColumn_version:
        res = sqlite3_bind_mstring(stmt, 1, game->version);
        break;
    case GamesColumn_developer:
        res = sqlite3_bind_mstring(stmt, 1, game->developer);
        break;
    case GamesColumn_type:
        res = sqlite3_bind_int(stmt, 1, game->type);
        break;
    case GamesColumn_status:
        res = sqlite3_bind_int(stmt, 1, game->status);
        break;
    case GamesColumn_url:
        res = sqlite3_bind_mstring(stmt, 1, game->url);
        break;
    case GamesColumn_added_on:
        res = sqlite3_bind_int64(stmt, 1, game->added_on);
        break;
    case GamesColumn_last_updated:
        res = sqlite3_bind_int64(stmt, 1, game->last_updated);
        break;
    case GamesColumn_last_full_check:
        res = sqlite3_bind_int64(stmt, 1, game->last_full_check);
        break;
    case GamesColumn_last_check_version:
        res = sqlite3_bind_mstring(stmt, 1, game->last_check_version);
        break;
    case GamesColumn_last_launched:
        res = sqlite3_bind_int64(stmt, 1, game->last_launched);
        break;
    case GamesColumn_score:
        res = sqlite3_bind_int(stmt, 1, game->score);
        break;
    case GamesColumn_votes:
        res = sqlite3_bind_int(stmt, 1, game->votes);
        break;
    case GamesColumn_rating:
        res = sqlite3_bind_int(stmt, 1, game->rating);
        break;
    case GamesColumn_finished:
        res = sqlite3_bind_mstring(stmt, 1, game->finished);
        break;
    case GamesColumn_installed:
        res = sqlite3_bind_mstring(stmt, 1, game->installed);
        break;
    case GamesColumn_updated:
        res = sqlite3_bind_int(stmt, 1, game->updated);
        break;
    case GamesColumn_archived:
        res = sqlite3_bind_int(stmt, 1, game->archived);
        break;
    case GamesColumn_executables:
        json_object* executables_json =
            json_object_new_array_ext(MstringList_size(game->executables));
        for
            M_EACH(executable, game->executables, MstringList_t) {
                json_object_array_add(
                    executables_json,
                    json_object_new_string(m_string_get_cstr(*executable)));
            }
        res = sqlite3_bind_json(stmt, 1, executables_json);
        json_object_put(executables_json);
        break;
    case GamesColumn_description:
        res = sqlite3_bind_mstring(stmt, 1, game->description);
        break;
    case GamesColumn_changelog:
        res = sqlite3_bind_mstring(stmt, 1, game->changelog);
        break;
    case GamesColumn_tags:
        json_object* tags_json = json_object_new_array();
        for(GameTag tag = GameTag_min(); tag <= GameTag_max(); tag++) {
            if(m_bitset_get(game->tags, tag) == false) continue;
            json_object_array_add(tags_json, json_object_new_int(tag));
        }
        res = sqlite3_bind_json(stmt, 1, tags_json);
        json_object_put(tags_json);
        break;
    case GamesColumn_unknown_tags:
        json_object* unknown_tags_json =
            json_object_new_array_ext(MstringList_size(game->unknown_tags));
        for
            M_EACH(unknown_tag, game->unknown_tags, MstringList_t) {
                json_object_array_add(
                    unknown_tags_json,
                    json_object_new_string(m_string_get_cstr(*unknown_tag)));
            }
        res = sqlite3_bind_json(stmt, 1, unknown_tags_json);
        json_object_put(unknown_tags_json);
        break;
    case GamesColumn_unknown_tags_flag:
        res = sqlite3_bind_int(stmt, 1, game->unknown_tags_flag);
        break;
    case GamesColumn_labels:
        // res = sqlite3_bind_int(stmt, 1, game->labels);
        break;
    case GamesColumn_tab:
        if(game->tab == NULL) {
            res = sqlite3_bind_null(stmt, 1);
        } else {
            res = sqlite3_bind_int(stmt, 1, game->tab->id);
        }
        break;
    case GamesColumn_notes:
        res = sqlite3_bind_mstring(stmt, 1, game->notes);
        break;
    case GamesColumn_image_url:
        res = sqlite3_bind_mstring(stmt, 1, game->image_url);
        break;
    case GamesColumn_previews_urls:
        json_object* previews_urls_json =
            json_object_new_array_ext(MstringList_size(game->previews_urls));
        for
            M_EACH(preview_url, game->previews_urls, MstringList_t) {
                json_object_array_add(
                    previews_urls_json,
                    json_object_new_string(m_string_get_cstr(*preview_url)));
            }
        res = sqlite3_bind_json(stmt, 1, previews_urls_json);
        json_object_put(previews_urls_json);
        break;
    case GamesColumn_downloads:
        // res = sqlite3_bind_int(stmt, 1, game->downloads);
        break;
    case GamesColumn_reviews_total:
        res = sqlite3_bind_int(stmt, 1, game->reviews_total);
        break;
    case GamesColumn_reviews:
        // res = sqlite3_bind_int(stmt, 1, game->reviews);
        break;
    }
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_*()");

    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_DONE, "sqlite3_step()");

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
}

Game* db_do_create_game(Db* db, GameDict_t* games, GameId id) {
    bool custom = id < 0;
    if(custom) {
        id = -1;
        for
            M_EACH(pair, *games, GameDict_t) {
                if(pair->key <= id) {
                    id = pair->key - 1;
                }
            }
        assert(id < 0);
    }

    int32_t res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(
        sql,
        "INSERT INTO %s (id,custom,name,added_on) VALUES (?,?,?,?) RETURNING ",
        games_table.name);
    db_append_column_names(&sql, &games_table);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    // id
    res = sqlite3_bind_int(stmt, 1, id);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    // custom
    res = sqlite3_bind_int(stmt, 2, custom);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    // name
    m_string_t name;
    m_string_init_printf(name, "%s (%d)", custom ? "Custom" : "Unknown", id);
    res = sqlite3_bind_mstring(stmt, 3, name);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_mstring()");
    m_string_clear(name);

    // added_on
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    res = sqlite3_bind_int64(stmt, 4, ts.tv_sec);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    assert(sqlite3_column_count(stmt) == games_table.columns_count);
    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_ROW, "sqlite3_step()");

    Game* game = game_init();
    db_parse_game(db, stmt, game);
    GameDict_set_at(*games, game->id, game);

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
    return game;
}

void db_do_delete_game(Db* db, Game* game, GameDict_t* games) {
    GameId id = game->id;
    bool removed = GameDict_erase(*games, id);
    game_free(game);
    assert(removed);
    UNUSED(removed);

    int32_t res;
    m_string_t sql;
    m_string_init(sql);

    m_string_printf(sql, "DELETE FROM %s WHERE id=?", games_table.name);
    sqlite3_stmt* stmt;
    res = sqlite3_prepare_v2(db->conn, m_string_get_cstr(sql), -1, &stmt, NULL);
    db_assert(db, res, SQLITE_OK, "sqlite3_prepare_v2()");

    res = sqlite3_bind_int(stmt, 1, id);
    db_assert(db, res, SQLITE_OK, "sqlite3_bind_int()");

    res = sqlite3_step(stmt);
    db_assert(db, res, SQLITE_DONE, "sqlite3_step()");

    res = sqlite3_finalize(stmt);
    db_assert(db, res, SQLITE_OK, "sqlite3_finalize()");

    m_string_clear(sql);
}
