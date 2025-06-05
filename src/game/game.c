#include "game.h"

/*
 * From: https://github.com/P-p-H-d/mlib/blob/5c57ad1f5185446842210934d022266f926a8aeb/example/ex-dict05.c
 * Provide OOR methods for OA Hashmap.
 * See documentation for definition of OOR
 * OOR values are represented as:
 * empty (0) is INT_MIN
 * deleted (1) is INT_MIN + 1
 * So valid range for integers are [INT_MIN+2, INT_MAX]
 */
bool GameId_oor_equal(GameId id, char oor) {
    return id == INT32_MIN + oor;
}

GameId GameId_oor_set(char oor) {
    return INT32_MIN + oor;
}

size_t GameId_hash(GameId id) {
    return (size_t)id; // Identity hash!
}

Game* game_init(void) {
    Game* game = malloc(sizeof(Game));

    m_string_init(game->name);
    m_string_init(game->version);
    m_string_init(game->developer);
    m_string_init(game->url);
    m_string_init(game->last_check_version);
    m_string_init(game->finished);
    m_string_init(game->installed);

    MstringList_init(game->executables);

    m_string_init(game->description);
    m_string_init(game->changelog);

    m_bitset_init(game->tags);
    m_bitset_resize(game->tags, 1 + GameTag_COUNT);

    MstringList_init(game->unknown_tags);

    LabelPtrList_init(game->labels);

    m_string_init(game->notes);
    m_string_init(game->image_url);

    MstringList_init(game->previews_urls);

    GameReviewList_init(game->reviews);

    return game;
}

void game_free(Game* game) {
    m_string_clear(game->name);
    m_string_clear(game->version);
    m_string_clear(game->developer);
    m_string_clear(game->url);
    m_string_clear(game->last_check_version);
    m_string_clear(game->finished);
    m_string_clear(game->installed);

    MstringList_clear(game->executables);

    m_string_clear(game->description);
    m_string_clear(game->changelog);

    m_bitset_clear(game->tags);

    MstringList_clear(game->unknown_tags);

    LabelPtrList_clear(game->labels);

    m_string_clear(game->notes);
    m_string_clear(game->image_url);

    MstringList_clear(game->previews_urls);

    GameReviewList_clear(game->reviews);

    free(game);
}
