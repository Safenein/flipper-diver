#include "view.h"

#include <stdio.h>

#include "flipper_diver_icons.h"

/* Provisional text rendering (task 8 of specs/001-regles-du-jeu); tasks 12
 * and 13 replace it with the final screens. */

/* One 16x16 icon per stratagem, in the order of stratagems[] (stratagems.c).
 * Sources: tools/make_icons.py. One entry per line, like stratagems.c. */
/* clang-format off */
static const Icon* const stratagem_icons[] = {
    &I_reinforce_16x16,
    &I_resupply_16x16,
    &I_sos_beacon_16x16,
    &I_hellbomb_16x16,
    &I_eagle_rearm_16x16,
    &I_eagle_500kg_16x16,
    &I_eagle_airstrike_16x16,
    &I_eagle_strafing_run_16x16,
    &I_orbital_laser_16x16,
    &I_orbital_precision_16x16,
    &I_orbital_railcannon_16x16,
    &I_orbital_380mm_16x16,
    &I_machine_gun_16x16,
    &I_autocannon_16x16,
    &I_railgun_16x16,
    &I_quasar_cannon_16x16,
    &I_recoilless_rifle_16x16,
    &I_shield_pack_16x16,
    &I_guard_dog_16x16,
    &I_patriot_exosuit_16x16,
};
/* clang-format on */

/* REQ-001-14: the build fails if a stratagem has no icon. */
_Static_assert(
    COUNT_OF(stratagem_icons) == STRATAGEM_COUNT,
    "every stratagem needs an icon in stratagem_icons");

static void draw_center(Canvas* canvas, int32_t y, const char* text) {
    canvas_draw_str_aligned(canvas, 64, y, AlignCenter, AlignTop, text);
}

static void draw_title(Canvas* canvas, const Game* game) {
    char line[32];
    canvas_set_font(canvas, FontPrimary);
    draw_center(canvas, 4, "Flipper Diver");
    canvas_set_font(canvas, FontSecondary);
    snprintf(
        line,
        sizeof(line),
        "Record: %lu (M%lu)",
        (unsigned long)game->best_score,
        (unsigned long)game->best_round);
    draw_center(canvas, 26, line);
    draw_center(canvas, 50, "OK pour jouer");
}

static void draw_playing(Canvas* canvas, const Game* game) {
    char line[32];
    const Stratagem* stratagem = game_current_stratagem(game);

    canvas_set_font(canvas, FontSecondary);
    if(stratagem) {
        draw_center(canvas, 2, stratagem->name);
        /* Entered arrows, a bar, then the arrows still to enter. */
        snprintf(
            line,
            sizeof(line),
            "%.*s|%s",
            (int)game->progress,
            stratagem->sequence,
            stratagem->sequence + game->progress);
        canvas_set_font(canvas, FontPrimary);
        draw_center(canvas, 18, line);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_icon(canvas, 0, 0, stratagem_icons[game->round_stratagems[game->current]]);
    }
    snprintf(
        line,
        sizeof(line),
        "%lu.%lu s  M%lu  %lu",
        (unsigned long)(game->timer_ms / 1000),
        (unsigned long)(game->timer_ms % 1000 / 100),
        (unsigned long)game->round,
        (unsigned long)game->score);
    draw_center(canvas, 40, line);
    snprintf(line, sizeof(line), "%u/%u", (unsigned)game->current + 1, (unsigned)game->round_size);
    draw_center(canvas, 52, line);

    if(game->flash_ms) {
        canvas_set_color(canvas, ColorXOR);
        canvas_draw_box(canvas, 0, 0, 128, 64);
        canvas_set_color(canvas, ColorBlack);
    }
}

static void draw_round_end(Canvas* canvas, const Game* game) {
    char line[32];
    canvas_set_font(canvas, FontPrimary);
    snprintf(line, sizeof(line), "Manche %lu finie", (unsigned long)game->round);
    draw_center(canvas, 2, line);
    canvas_set_font(canvas, FontSecondary);
    snprintf(line, sizeof(line), "Manche +%lu", (unsigned long)game->round_bonus);
    draw_center(canvas, 18, line);
    snprintf(line, sizeof(line), "Temps +%lu", (unsigned long)game->time_bonus);
    draw_center(canvas, 28, line);
    snprintf(line, sizeof(line), "Parfait +%lu", (unsigned long)game->perfect_bonus);
    draw_center(canvas, 38, line);
    snprintf(line, sizeof(line), "Score %lu", (unsigned long)game->score);
    draw_center(canvas, 50, line);
}

static void draw_game_over(Canvas* canvas, const Game* game) {
    char line[32];
    canvas_set_font(canvas, FontPrimary);
    draw_center(canvas, 2, "Game over");
    canvas_set_font(canvas, FontSecondary);
    snprintf(
        line,
        sizeof(line),
        "Score %lu  M%lu",
        (unsigned long)game->score,
        (unsigned long)game->round);
    draw_center(canvas, 20, line);
    snprintf(line, sizeof(line), "Record %lu", (unsigned long)game->best_score);
    draw_center(canvas, 32, line);
    draw_center(canvas, 50, "OK: titre  Back: quitter");
}

void view_draw(Canvas* canvas, const Game* game) {
    char line[32];
    canvas_clear(canvas);

    switch(game->phase) {
    case GamePhaseTitle:
        draw_title(canvas, game);
        break;
    case GamePhaseAnnounce:
        canvas_set_font(canvas, FontPrimary);
        snprintf(line, sizeof(line), "Manche %lu", (unsigned long)game->round);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignCenter, line);
        break;
    case GamePhasePlaying:
        draw_playing(canvas, game);
        break;
    case GamePhaseRoundEnd:
        draw_round_end(canvas, game);
        break;
    case GamePhasePaused:
        canvas_set_font(canvas, FontPrimary);
        draw_center(canvas, 10, "Pause");
        canvas_set_font(canvas, FontSecondary);
        draw_center(canvas, 34, "OK: reprendre");
        draw_center(canvas, 46, "Back: quitter");
        break;
    case GamePhaseGameOver:
        draw_game_over(canvas, game);
        break;
    }
}
