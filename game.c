#include "game.h"

#include <stddef.h>

#define NO_STRATAGEM 0xFFu

static uint32_t rng_next(Game* game) {
    /* xorshift32 (Marsaglia): deterministic on every platform. */
    uint32_t x = game->rng;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    game->rng = x;
    return x;
}

/* Draws a stratagem different from the previous one, without rejection
 * loop: a repeat is shifted to the next entry of the table. */
static uint8_t draw_stratagem(Game* game) {
    uint8_t index = (uint8_t)(rng_next(game) % STRATAGEM_COUNT);
    if(index == game->last_drawn) {
        index = (uint8_t)((index + 1u) % STRATAGEM_COUNT);
    }
    game->last_drawn = index;
    return index;
}

static void start_round(Game* game, uint32_t round) {
    game->round = round;
    game->round_size = game_round_size(round);
    for(uint8_t i = 0; i < game->round_size; i++) {
        game->round_stratagems[i] = draw_stratagem(game);
    }
    game->current = 0;
    game->progress = 0;
    game->perfect = true;
    game->timer_ms = GAME_TIMER_MAX_MS;
    game->flash_ms = 0;
    game->phase = GamePhaseAnnounce;
    game->phase_ms = GAME_ANNOUNCE_MS;
}

/* Called once the last stratagem of the round is done, after its refill. */
static GameEvents finish_round(Game* game) {
    game->round_bonus = GAME_ROUND_BONUS_BASE + GAME_ROUND_BONUS_STEP * (game->round - 1);
    game->time_bonus = game->timer_ms / 100u;
    if(game->time_bonus > GAME_TIME_BONUS_MAX) {
        game->time_bonus = GAME_TIME_BONUS_MAX;
    }
    game->perfect_bonus = game->perfect ? GAME_PERFECT_BONUS : 0;
    game->score += game->round_bonus + game->time_bonus + game->perfect_bonus;
    game->phase = GamePhaseRoundEnd;
    game->phase_ms = GAME_ROUND_END_MS;
    return GameEventRoundDone;
}

static GameEvents game_over(Game* game) {
    game->phase = GamePhaseGameOver;
    if(game->score > game->best_score) {
        game->best_score = game->score;
        game->best_round = game->round;
        return GameEventGameOver | GameEventNewRecord;
    }
    return GameEventGameOver;
}

_Static_assert(
    (int)GameKeyUp == (int)StratagemArrowUp && (int)GameKeyDown == (int)StratagemArrowDown &&
        (int)GameKeyLeft == (int)StratagemArrowLeft &&
        (int)GameKeyRight == (int)StratagemArrowRight,
    "arrow keys must map one to one onto stratagem arrows");

static GameEvents enter_arrow(Game* game, GameKey key) {
    if(game->current >= game->round_size) {
        return GameEventNone;
    }
    const Stratagem* stratagem = &stratagems[game->round_stratagems[game->current]];

    if(stratagem_arrow(stratagem, game->progress) != (StratagemArrow)key) {
        game->progress = 0;
        game->perfect = false;
        game->flash_ms = GAME_ERROR_FLASH_MS;
        return GameEventError;
    }

    game->progress++;
    size_t length = stratagem_length(stratagem);
    if(game->progress < length) {
        return GameEventArrowOk;
    }

    game->score += GAME_POINTS_PER_ARROW * (uint32_t)length;
    game->timer_ms += GAME_TIMER_REFILL_MS;
    if(game->timer_ms > GAME_TIMER_MAX_MS) {
        game->timer_ms = GAME_TIMER_MAX_MS;
    }
    game->current++;
    game->progress = 0;
    GameEvents events = GameEventArrowOk | GameEventStratagemDone;
    if(game->current == game->round_size) {
        events |= finish_round(game);
    }
    return events;
}

static void start_game(Game* game) {
    game->score = 0;
    game->round_bonus = 0;
    game->time_bonus = 0;
    game->perfect_bonus = 0;
    start_round(game, 1);
}

void game_init(Game* game, uint32_t seed, uint32_t best_score, uint32_t best_round) {
    *game = (Game){
        .phase = GamePhaseTitle,
        .rng = seed ? seed : 0x9E3779B9u,
        .last_drawn = NO_STRATAGEM,
        .best_score = best_score,
        .best_round = best_round,
    };
}

uint8_t game_round_size(uint32_t round) {
    if(round >= GAME_ROUND_SIZE_MAX - GAME_ROUND_SIZE_BASE) {
        return GAME_ROUND_SIZE_MAX;
    }
    return (uint8_t)(GAME_ROUND_SIZE_BASE + round);
}

const Stratagem* game_current_stratagem(const Game* game) {
    switch(game->phase) {
    case GamePhaseAnnounce:
    case GamePhasePlaying:
    case GamePhaseRoundEnd:
        if(game->current < game->round_size) {
            return &stratagems[game->round_stratagems[game->current]];
        }
        return NULL;
    default:
        return NULL;
    }
}

GameEvents game_advance(Game* game, uint32_t elapsed_ms) {
    GameEvents events = GameEventNone;

    if(game->phase != GamePhasePaused) {
        game->flash_ms = elapsed_ms < game->flash_ms ? game->flash_ms - elapsed_ms : 0;
    }

    /* Loops so that time left over when a phase ends carries into the next
     * one: the result does not depend on how the caller slices time. */
    while(elapsed_ms > 0) {
        if(game->phase == GamePhaseAnnounce) {
            if(elapsed_ms < game->phase_ms) {
                game->phase_ms -= elapsed_ms;
                break;
            }
            elapsed_ms -= game->phase_ms;
            game->phase_ms = 0;
            game->phase = GamePhasePlaying;
        } else if(game->phase == GamePhasePlaying) {
            if(elapsed_ms < game->timer_ms) {
                game->timer_ms -= elapsed_ms;
                break;
            }
            game->timer_ms = 0;
            events |= game_over(game);
            break;
        } else if(game->phase == GamePhaseRoundEnd) {
            if(elapsed_ms < game->phase_ms) {
                game->phase_ms -= elapsed_ms;
                break;
            }
            elapsed_ms -= game->phase_ms;
            start_round(game, game->round + 1);
        } else {
            break;
        }
    }

    return events;
}

GameEvents game_key(Game* game, GameKey key) {
    if(key == GameKeyBack) {
        switch(game->phase) {
        case GamePhaseAnnounce:
        case GamePhasePlaying:
        case GamePhaseRoundEnd:
            game_pause(game);
            return GameEventNone;
        case GamePhasePaused:
            /* Leaving mid-game: the game is not over, no record. */
            return GameEventQuit;
        default:
            break;
        }
    }

    switch(game->phase) {
    case GamePhaseTitle:
        if(key == GameKeyOk) {
            start_game(game);
        } else if(key == GameKeyBack) {
            return GameEventQuit;
        }
        break;
    case GamePhaseAnnounce:
        /* Arrows are ignored until the first stratagem shows. */
        break;
    case GamePhasePlaying:
        if(key <= GameKeyRight) {
            return enter_arrow(game, key);
        }
        break;
    case GamePhaseRoundEnd:
        if(key == GameKeyOk) {
            start_round(game, game->round + 1);
        }
        break;
    case GamePhasePaused:
        if(key == GameKeyOk) {
            game->phase = game->paused_phase;
        }
        break;
    case GamePhaseGameOver:
        if(key == GameKeyOk) {
            game->phase = GamePhaseTitle;
        } else if(key == GameKeyBack) {
            return GameEventQuit;
        }
        break;
    default:
        break;
    }
    return GameEventNone;
}

void game_pause(Game* game) {
    switch(game->phase) {
    case GamePhaseAnnounce:
    case GamePhasePlaying:
    case GamePhaseRoundEnd:
        /* Every countdown stops: game_advance() only runs them in these
         * phases, so resuming finds the exact same state. */
        game->paused_phase = game->phase;
        game->phase = GamePhasePaused;
        break;
    default:
        break;
    }
}
