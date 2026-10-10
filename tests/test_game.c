#include "test.h"

#include <string.h>

#include "game.h"

#define SEED 12345u

/* Title -> Ok: a new game waiting on the round 1 announce. */
static void start(Game* game) {
    game_init(game, SEED, 0, 0);
    game_key(game, GameKeyOk);
}

/* Title -> Ok -> announce elapsed: first stratagem active, full timer. */
static void start_playing(Game* game) {
    start(game);
    game_advance(game, GAME_ANNOUNCE_MS);
}

static uint8_t stratagem_index(const char* name) {
    for(uint8_t i = 0; i < STRATAGEM_COUNT; i++) {
        if(strcmp(stratagems[i].name, name) == 0) {
            return i;
        }
    }
    CHECK(!"stratagem not found");
    return 0;
}

/* Makes the named stratagem the active one. */
static void force_current(Game* game, const char* name) {
    game->round_stratagems[game->current] = stratagem_index(name);
}

static GameKey arrow_key(char letter) {
    switch(letter) {
    case 'U':
        return GameKeyUp;
    case 'D':
        return GameKeyDown;
    case 'L':
        return GameKeyLeft;
    default:
        return GameKeyRight;
    }
}

/* Enters the arrows given as letters; returns the events of the last one. */
static GameEvents type(Game* game, const char* letters) {
    GameEvents events = GameEventNone;
    for(const char* c = letters; *c; c++) {
        events = game_key(game, arrow_key(*c));
    }
    return events;
}

/* Enters the whole sequence of the active stratagem. */
static GameEvents complete_current(Game* game) {
    return type(game, game_current_stratagem(game)->sequence);
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

/* REQ-001-6: Eagle 500kg Bomb is URDDD. */
static void test_correct_arrow_advances(void) {
    Game game;
    start_playing(&game);
    force_current(&game, "Eagle 500kg Bomb");
    CHECK_EQ(type(&game, "U"), GameEventArrowOk);
    CHECK_EQ(game.progress, 1);
}

/* REQ-001-7: wrong arrow restarts the sequence, timer untouched. */
static void test_wrong_arrow_restarts_sequence(void) {
    Game game;
    start_playing(&game);
    force_current(&game, "Eagle 500kg Bomb");
    game_advance(&game, 2000);
    type(&game, "U");
    CHECK_EQ(type(&game, "D"), GameEventError);
    CHECK_EQ(game.progress, 0);
    CHECK(!game.perfect);
    CHECK_EQ(game.timer_ms, 8000);
    CHECK_EQ(game.flash_ms, GAME_ERROR_FLASH_MS);
}

static void test_wrong_first_arrow_is_an_error(void) {
    Game game;
    start_playing(&game);
    force_current(&game, "Eagle 500kg Bomb");
    CHECK_EQ(type(&game, "L"), GameEventError);
    CHECK(!game.perfect);
}

static void test_error_flash_fades(void) {
    Game game;
    start_playing(&game);
    force_current(&game, "Eagle 500kg Bomb");
    type(&game, "L");
    game_advance(&game, GAME_ERROR_FLASH_MS - 1);
    CHECK_EQ(game.flash_ms, 1);
    game_advance(&game, 1);
    CHECK_EQ(game.flash_ms, 0);
}

/* REQ-001-8: 6 arrows with 4.2 s left -> +30 points, 5.7 s. */
static void test_stratagem_done_scores_and_refills(void) {
    Game game;
    start_playing(&game);
    force_current(&game, "Autocannon");
    game_advance(&game, 10000 - 4200);
    CHECK_EQ(game.timer_ms, 4200);
    GameEvents events = complete_current(&game);
    CHECK(events & GameEventStratagemDone);
    CHECK_EQ(game.score, 30);
    CHECK_EQ(game.timer_ms, 5700);
    CHECK_EQ(game.current, 1);
    CHECK_EQ(game.progress, 0);
}

/* REQ-001-8: refill capped at 10 s. */
static void test_refill_is_capped(void) {
    Game game;
    start_playing(&game);
    game_advance(&game, 900);
    complete_current(&game);
    CHECK_EQ(game.timer_ms, 10000);
}

/* REQ-001-9: the timer counts down while playing. */
static void test_timer_counts_down(void) {
    Game game;
    start_playing(&game);
    game_advance(&game, 1000);
    CHECK_EQ(game.timer_ms, 9000);
}

/* Time left over at the end of the announce carries into the round. */
static void test_announce_overflow_carries_into_timer(void) {
    Game game;
    start(&game);
    game_advance(&game, GAME_ANNOUNCE_MS + 300);
    CHECK_EQ(game.phase, GamePhasePlaying);
    CHECK_EQ(game.timer_ms, 9700);
}

/* REQ-001-10: timer at 0 with one arrow left -> game over, no points. */
static void test_timer_zero_is_game_over(void) {
    Game game;
    start_playing(&game);
    force_current(&game, "Eagle Strafing Run"); /* URR */
    type(&game, "UR");
    CHECK_EQ(game_advance(&game, 10000), GameEventGameOver);
    CHECK_EQ(game.phase, GamePhaseGameOver);
    CHECK_EQ(game.timer_ms, 0);
    CHECK_EQ(type(&game, "R"), GameEventNone);
    CHECK_EQ(game.score, 0);
}

/* Puts the game on the last stratagem of round n, playing. */
static void last_stratagem_of_round(Game* game, uint32_t round) {
    start_playing(game);
    game->round = round;
    game->round_size = game_round_size(round);
    game->current = game->round_size - 1;
}

/* REQ-001-11: round 3, no error, 4 567 ms left after the last refill. */
static void test_round_bonuses_perfect(void) {
    Game game;
    last_stratagem_of_round(&game, 3);
    force_current(&game, "Resupply"); /* 4 arrows: 20 points */
    game.timer_ms = 4567 - GAME_TIMER_REFILL_MS;
    GameEvents events = complete_current(&game);
    CHECK(events & GameEventRoundDone);
    CHECK_EQ(game.round_bonus, 125);
    CHECK_EQ(game.time_bonus, 45);
    CHECK_EQ(game.perfect_bonus, 100);
    CHECK_EQ(game.score, 20 + 125 + 45 + 100);
    CHECK_EQ(game.phase, GamePhaseRoundEnd);
}

/* REQ-001-11: round 1 with an error, 10 000 ms left. */
static void test_round_bonuses_with_error(void) {
    Game game;
    last_stratagem_of_round(&game, 1);
    force_current(&game, "Resupply");
    type(&game, "U"); /* error */
    complete_current(&game);
    CHECK_EQ(game.round_bonus, 75);
    CHECK_EQ(game.time_bonus, 100);
    CHECK_EQ(game.perfect_bonus, 0);
}

/* REQ-001-12: next round announce after 3 s, or at once with Ok. */
static void test_round_end_lasts_3_s(void) {
    Game game;
    last_stratagem_of_round(&game, 1);
    complete_current(&game);
    game_advance(&game, GAME_ROUND_END_MS - 1);
    CHECK_EQ(game.phase, GamePhaseRoundEnd);
    game_advance(&game, 1);
    CHECK_EQ(game.phase, GamePhaseAnnounce);
    CHECK_EQ(game.round, 2);
    CHECK_EQ(game.round_size, 8);
    CHECK_EQ(game.timer_ms, 10000);
    CHECK(game.perfect);
}

static void test_round_end_ok_skips_wait(void) {
    Game game;
    last_stratagem_of_round(&game, 1);
    complete_current(&game);
    game_key(&game, GameKeyOk);
    CHECK_EQ(game.phase, GamePhaseAnnounce);
    CHECK_EQ(game.round, 2);
}

/* REQ-001-4, across rounds: 10 000 rounds played through, no stratagem
 * follows itself, including from one round to the next. */
static void test_no_consecutive_repeat_over_10000_rounds(void) {
    Game game;
    start(&game);
    int previous = -1;
    for(uint32_t round = 1; round <= 10000; round++) {
        CHECK_EQ(game.round, round);
        CHECK_EQ(game.round_size, game_round_size(round));
        game_advance(&game, GAME_ANNOUNCE_MS);
        for(uint8_t i = 0; i < game.round_size; i++) {
            int index = game.round_stratagems[i];
            CHECK(index != previous);
            previous = index;
            complete_current(&game);
        }
        CHECK_EQ(game.phase, GamePhaseRoundEnd);
        game_key(&game, GameKeyOk);
        if(test_failures) {
            break;
        }
    }
}

/* REQ-001-16, 17: pause at 2/5 arrows and 6.3 s -> identical on resume. */
static void test_pause_freezes_and_resumes_identically(void) {
    Game game;
    start_playing(&game);
    force_current(&game, "Eagle 500kg Bomb");
    game_advance(&game, 10000 - 6300);
    type(&game, "UR");
    game_key(&game, GameKeyBack);
    CHECK_EQ(game.phase, GamePhasePaused);
    CHECK(game_current_stratagem(&game) == NULL);
    game_advance(&game, 2000);
    CHECK_EQ(type(&game, "DD"), GameEventNone);
    game_key(&game, GameKeyOk);
    CHECK_EQ(game.phase, GamePhasePlaying);
    CHECK_EQ(game.progress, 2);
    CHECK_EQ(game.timer_ms, 6300);
}

static void test_pause_during_announce_and_round_end(void) {
    Game game;
    start(&game);
    game_advance(&game, 500);
    game_key(&game, GameKeyBack);
    game_advance(&game, 5000);
    game_key(&game, GameKeyOk);
    CHECK_EQ(game.phase, GamePhaseAnnounce);
    CHECK_EQ(game.phase_ms, GAME_ANNOUNCE_MS - 500);

    last_stratagem_of_round(&game, 1);
    complete_current(&game);
    game_advance(&game, 1000);
    game_key(&game, GameKeyBack);
    game_advance(&game, 5000);
    game_key(&game, GameKeyOk);
    CHECK_EQ(game.phase, GamePhaseRoundEnd);
    CHECK_EQ(game.phase_ms, GAME_ROUND_END_MS - 1000);
}

/* REQ-001-17: Back while paused quits, the record stays. */
static void test_back_in_pause_quits_without_record(void) {
    Game game;
    game_init(&game, SEED, 1000, 2);
    game_key(&game, GameKeyOk);
    game.score = 5000;
    game_key(&game, GameKeyBack);
    CHECK_EQ(game_key(&game, GameKeyBack), GameEventQuit);
    CHECK_EQ(game.best_score, 1000);
    CHECK_EQ(game.best_round, 2);
}

/* REQ-001-18 (logic): game_pause() only acts while a round runs. */
static void test_game_pause_outside_round_does_nothing(void) {
    Game game;
    game_init(&game, SEED, 0, 0);
    game_pause(&game);
    CHECK_EQ(game.phase, GamePhaseTitle);

    start_playing(&game);
    game_pause(&game);
    CHECK_EQ(game.phase, GamePhasePaused);
    game_pause(&game);
    CHECK_EQ(game.paused_phase, GamePhasePlaying);
}

int main(void) {
    RUN_TEST(test_init_shows_title_with_record);
    RUN_TEST(test_title_ok_starts_round_1_with_score_0);
    RUN_TEST(test_title_back_quits);
    RUN_TEST(test_round_sizes);
    RUN_TEST(test_announce_ignores_arrows);
    RUN_TEST(test_announce_lasts_1500_ms_then_full_timer);
    RUN_TEST(test_no_consecutive_repeat_in_first_round);
    RUN_TEST(test_correct_arrow_advances);
    RUN_TEST(test_wrong_arrow_restarts_sequence);
    RUN_TEST(test_wrong_first_arrow_is_an_error);
    RUN_TEST(test_error_flash_fades);
    RUN_TEST(test_stratagem_done_scores_and_refills);
    RUN_TEST(test_refill_is_capped);
    RUN_TEST(test_timer_counts_down);
    RUN_TEST(test_announce_overflow_carries_into_timer);
    RUN_TEST(test_timer_zero_is_game_over);
    RUN_TEST(test_round_bonuses_perfect);
    RUN_TEST(test_round_bonuses_with_error);
    RUN_TEST(test_round_end_lasts_3_s);
    RUN_TEST(test_round_end_ok_skips_wait);
    RUN_TEST(test_no_consecutive_repeat_over_10000_rounds);
    RUN_TEST(test_pause_freezes_and_resumes_identically);
    RUN_TEST(test_pause_during_announce_and_round_end);
    RUN_TEST(test_back_in_pause_quits_without_record);
    RUN_TEST(test_game_pause_outside_round_does_nothing);
    return test_report();
}
