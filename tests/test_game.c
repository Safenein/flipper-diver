#include "test.h"

#include "game.h"

#define SEED 12345u

/* Title -> Ok: a new game waiting on the round 1 announce. */
static void start(Game* game) {
    game_init(game, SEED, 0, 0);
    game_key(game, GameKeyOk);
}

static void test_init_shows_title_with_record(void) {
    Game game;
    game_init(&game, SEED, 1250, 4);
    CHECK_EQ(game.phase, GamePhaseTitle);
    CHECK_EQ(game.best_score, 1250);
    CHECK_EQ(game.best_round, 4);
    CHECK(game_current_stratagem(&game) == NULL);
}

static void test_title_ok_starts_round_1_with_score_0(void) {
    Game game;
    start(&game);
    CHECK_EQ(game.phase, GamePhaseAnnounce);
    CHECK_EQ(game.round, 1);
    CHECK_EQ(game.score, 0);
}

static void test_title_back_quits(void) {
    Game game;
    game_init(&game, SEED, 0, 0);
    CHECK_EQ(game_key(&game, GameKeyBack), GameEventQuit);
}

static void test_round_sizes(void) {
    CHECK_EQ(game_round_size(1), 7);
    CHECK_EQ(game_round_size(5), 11);
    CHECK_EQ(game_round_size(9), 15);
    CHECK_EQ(game_round_size(10), 16);
    CHECK_EQ(game_round_size(25), 16);
    CHECK_EQ(game_round_size(UINT32_MAX), 16);
}

static void test_announce_ignores_arrows(void) {
    Game game;
    start(&game);
    game_advance(&game, 1000);
    CHECK_EQ(game_key(&game, GameKeyUp), GameEventNone);
    CHECK_EQ(game_key(&game, GameKeyDown), GameEventNone);
    CHECK_EQ(game.phase, GamePhaseAnnounce);
    CHECK_EQ(game.progress, 0);
    CHECK(game.perfect);
}

static void test_announce_lasts_1500_ms_then_full_timer(void) {
    Game game;
    start(&game);
    game_advance(&game, 1499);
    CHECK_EQ(game.phase, GamePhaseAnnounce);
    game_advance(&game, 1);
    CHECK_EQ(game.phase, GamePhasePlaying);
    CHECK_EQ(game.timer_ms, 10000);
    CHECK(game_current_stratagem(&game) == &stratagems[game.round_stratagems[0]]);
}

/* REQ-001-4, within a round: over many seeds, no stratagem follows itself. */
static void test_no_consecutive_repeat_in_first_round(void) {
    for(uint32_t seed = 0; seed < 10000; seed++) {
        Game game;
        game_init(&game, seed, 0, 0);
        game_key(&game, GameKeyOk);
        CHECK_EQ(game.round_size, 7);
        for(uint8_t i = 0; i < game.round_size; i++) {
            CHECK(game.round_stratagems[i] < STRATAGEM_COUNT);
            if(i > 0) {
                CHECK(game.round_stratagems[i] != game.round_stratagems[i - 1]);
            }
        }
    }
}

int main(void) {
    RUN_TEST(test_init_shows_title_with_record);
    RUN_TEST(test_title_ok_starts_round_1_with_score_0);
    RUN_TEST(test_title_back_quits);
    RUN_TEST(test_round_sizes);
    RUN_TEST(test_announce_ignores_arrows);
    RUN_TEST(test_announce_lasts_1500_ms_then_full_timer);
    RUN_TEST(test_no_consecutive_repeat_in_first_round);
    return test_report();
}
