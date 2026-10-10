/* Game rules of Stratagem Hero: phases, timer, scoring and round building.
 * Pure C, no Furi (ADR-0004): the caller feeds elapsed time and keys, and
 * reacts to the returned GameEvents (notifications, saving the record).
 * See specs/001-regles-du-jeu/design.md. */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "stratagems.h"

#define GAME_TIMER_MAX_MS     10000u
#define GAME_TIMER_REFILL_MS  1500u
#define GAME_ANNOUNCE_MS      1500u
#define GAME_ROUND_END_MS     3000u
#define GAME_ERROR_FLASH_MS   120u
#define GAME_ROUND_SIZE_BASE  6u
#define GAME_ROUND_SIZE_MAX   16u
#define GAME_POINTS_PER_ARROW 5u
#define GAME_ROUND_BONUS_BASE 75u
#define GAME_ROUND_BONUS_STEP 25u
#define GAME_TIME_BONUS_MAX   100u
#define GAME_PERFECT_BONUS    100u

typedef enum {
    GamePhaseTitle,
    GamePhaseAnnounce,
    GamePhasePlaying,
    GamePhaseRoundEnd,
    GamePhasePaused,
    GamePhaseGameOver,
} GamePhase;

typedef enum {
    GameKeyUp,
    GameKeyDown,
    GameKeyLeft,
    GameKeyRight,
    GameKeyOk,
    GameKeyBack,
} GameKey;

/* Bit flags returned by game_advance() and game_key(). */
typedef enum {
    GameEventNone = 0,
    GameEventArrowOk = 1u << 0, /* expected arrow entered */
    GameEventError = 1u << 1, /* wrong arrow: sequence restarted */
    GameEventStratagemDone = 1u << 2,
    GameEventRoundDone = 1u << 3,
    GameEventGameOver = 1u << 4,
    GameEventNewRecord = 1u << 5, /* with GameOver: best_* were updated */
    GameEventQuit = 1u << 6, /* the app must exit */
} GameEvent;

typedef uint32_t GameEvents;

/* Game state. Allocated by the caller; only game.c writes it, other modules
 * read it (rendering) under the caller's lock. */
typedef struct {
    GamePhase phase;
    GamePhase paused_phase; /* phase to resume, while phase is Paused */

    uint32_t rng; /* xorshift32 state, never 0 */
    uint8_t last_drawn; /* index of the last stratagem drawn, for dedup */

    uint32_t score;
    uint32_t best_score;
    uint32_t best_round;
    bool new_record; /* the game that just ended beat the record */

    uint32_t round; /* 1-based, 0 before the first round */
    uint8_t round_size;
    uint8_t round_stratagems[GAME_ROUND_SIZE_MAX]; /* indices in stratagems[] */
    uint8_t current; /* index in round_stratagems of the active stratagem */
    uint8_t progress; /* arrows already entered for the active stratagem */
    bool perfect; /* no error so far in this round */

    uint32_t timer_ms; /* round timer, counts down while Playing */
    uint32_t phase_ms; /* time left in Announce or RoundEnd */
    uint32_t flash_ms; /* time left for the error flash */

    /* Bonuses of the last finished round, for the round end screen. */
    uint32_t round_bonus;
    uint32_t time_bonus;
    uint32_t perfect_bonus;
} Game;

/* Puts the game on the title screen. seed may be any value. */
void game_init(Game* game, uint32_t seed, uint32_t best_score, uint32_t best_round);

/* Lets elapsed_ms go by. Call it with the time elapsed since the last call
 * before every game_key(), so that the timer is up to date. */
GameEvents game_advance(Game* game, uint32_t elapsed_ms);

/* Applies one key press. Filtering of input types (press, short) is done by
 * the caller. */
GameEvents game_key(Game* game, GameKey key);

/* Pauses the game if a round is in progress; no effect otherwise. */
void game_pause(Game* game);

/* Number of stratagems in round n (n >= 1). */
uint8_t game_round_size(uint32_t round);

/* Active stratagem, or NULL outside Announce, Playing and RoundEnd. */
const Stratagem* game_current_stratagem(const Game* game);
