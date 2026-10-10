#include "view.h"

#include <stdio.h>

#include "flipper_diver_icons.h"

/* Screens of specs/001-regles-du-jeu/design.md (« Rendu »). Texts are
 * kept ASCII: the built-in fonts have no accented letters. */

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

/* Round screen layout (design.md, « Rendu »), in pixels. */
#define ICON_SIZE       16
#define ICON_Y          2
#define ICON_PITCH      24 /* between slot centers */
#define ICON_SLOTS_SIDE 2 /* done ones on the left, next ones on the right */
#define ACTIVE_FRAME    20
#define NAME_Y          22
#define ARROW_SIZE      12
#define ARROW_PITCH     14
#define ARROW_Y         34
#define GAUGE_X         0
#define GAUGE_Y         55
#define GAUGE_W         72
#define GAUGE_H         7
#define BOTTOM_TEXT_Y   63

/* Draws a horizontal span of an arrow given in the frame of an up arrow
 * (u across, v along, both 0..ARROW_SIZE-1), rotated to dir. */
static void arrow_span(
    Canvas* canvas,
    int32_t x,
    int32_t y,
    StratagemArrow dir,
    int32_t u1,
    int32_t u2,
    int32_t v) {
    const int32_t last = ARROW_SIZE - 1;
    switch(dir) {
    case StratagemArrowUp:
        canvas_draw_line(canvas, x + u1, y + v, x + u2, y + v);
        break;
    case StratagemArrowDown:
        canvas_draw_line(canvas, x + u1, y + last - v, x + u2, y + last - v);
        break;
    case StratagemArrowLeft:
        canvas_draw_line(canvas, x + v, y + u1, x + v, y + u2);
        break;
    case StratagemArrowRight:
        canvas_draw_line(canvas, x + last - v, y + u1, x + last - v, y + u2);
        break;
    }
}

/* A 12x12 arrow: triangular head on rows 0-5, stem 4 px wide on rows 6-11.
 * Filled once entered, outlined while still to enter. */
static void draw_arrow(Canvas* canvas, int32_t x, int32_t y, StratagemArrow dir, bool filled) {
    const int32_t mid = ARROW_SIZE / 2; /* 6 */
    for(int32_t v = 0; v < mid; v++) {
        int32_t u1 = mid - 1 - v;
        int32_t u2 = mid + v;
        if(filled || v == mid - 1) {
            arrow_span(canvas, x, y, dir, u1, u2, v);
        } else {
            arrow_span(canvas, x, y, dir, u1, u1, v);
            arrow_span(canvas, x, y, dir, u2, u2, v);
        }
    }
    for(int32_t v = mid; v < ARROW_SIZE; v++) {
        if(filled || v == ARROW_SIZE - 1) {
            arrow_span(canvas, x, y, dir, 4, 7, v);
        } else {
            arrow_span(canvas, x, y, dir, 4, 4, v);
            arrow_span(canvas, x, y, dir, 7, 7, v);
        }
    }
}

/* Row of icons: active one framed in the center, the 2 last done ones
 * crossed on its left, the 2 next ones on its right (REQ-001-13). */
static void draw_icon_row(Canvas* canvas, const Game* game) {
    for(int32_t slot = -ICON_SLOTS_SIDE; slot <= ICON_SLOTS_SIDE; slot++) {
        int32_t index = (int32_t)game->current + slot;
        if(index < 0 || index >= (int32_t)game->round_size) {
            continue;
        }
        int32_t x = 64 + slot * ICON_PITCH - ICON_SIZE / 2;
        canvas_draw_icon(canvas, x, ICON_Y, stratagem_icons[game->round_stratagems[index]]);
        if(slot < 0) {
            canvas_draw_line(canvas, x, ICON_Y, x + ICON_SIZE - 1, ICON_Y + ICON_SIZE - 1);
            canvas_draw_line(canvas, x + ICON_SIZE - 1, ICON_Y, x, ICON_Y + ICON_SIZE - 1);
        }
    }
    canvas_draw_frame(canvas, 64 - ACTIVE_FRAME / 2, 0, ACTIVE_FRAME, ACTIVE_FRAME);
}

static void draw_sequence(Canvas* canvas, const Game* game, const Stratagem* stratagem) {
    int32_t length = (int32_t)stratagem_length(stratagem);
    int32_t x = 64 - (length * ARROW_PITCH - (ARROW_PITCH - ARROW_SIZE)) / 2;
    for(int32_t i = 0; i < length; i++) {
        draw_arrow(
            canvas,
            x + i * ARROW_PITCH,
            ARROW_Y,
            stratagem_arrow(stratagem, (size_t)i),
            i < (int32_t)game->progress);
    }
}

/* Timer gauge on the left, round and score on the right. */
static void draw_bottom_bar(Canvas* canvas, const Game* game) {
    char line[24];
    canvas_draw_frame(canvas, GAUGE_X, GAUGE_Y, GAUGE_W, GAUGE_H);
    size_t fill = (size_t)((uint64_t)game->timer_ms * (GAUGE_W - 2) / GAME_TIMER_MAX_MS);
    if(fill) {
        canvas_draw_box(canvas, GAUGE_X + 1, GAUGE_Y + 1, fill, GAUGE_H - 2);
    }
    canvas_set_font(canvas, FontSecondary);
    snprintf(
        line, sizeof(line), "M%lu %lu", (unsigned long)game->round, (unsigned long)game->score);
    canvas_draw_str_aligned(canvas, 127, BOTTOM_TEXT_Y, AlignRight, AlignBottom, line);
}

static void draw_playing(Canvas* canvas, const Game* game) {
    const Stratagem* stratagem = game_current_stratagem(game);
    if(stratagem) {
        draw_icon_row(canvas, game);
        canvas_set_font(canvas, FontSecondary);
        draw_center(canvas, NAME_Y, stratagem->name);
        draw_sequence(canvas, game, stratagem);
    }
    draw_bottom_bar(canvas, game);

    /* Error flash (REQ-001-7): the whole screen inverted for 120 ms. */
    if(game->flash_ms) {
        canvas_set_color(canvas, ColorXOR);
        canvas_draw_box(canvas, 0, 0, 128, 64);
        canvas_set_color(canvas, ColorBlack);
    }
}

/* Footer line with the two key hints (Ok on the left, Back on the right). */
static void draw_hints(Canvas* canvas, const char* ok, const char* back) {
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 0, 63, AlignLeft, AlignBottom, ok);
    canvas_draw_str_aligned(canvas, 127, 63, AlignRight, AlignBottom, back);
}

static void draw_title(Canvas* canvas, const Game* game) {
    char line[40];
    canvas_set_font(canvas, FontPrimary);
    draw_center(canvas, 2, "FLIPPER DIVER");

    /* Decorative sequence: the Reinforce stratagem. */
    const Stratagem* reinforce = &stratagems[0];
    size_t length = stratagem_length(reinforce);
    int32_t x = 64 - (int32_t)(length * ARROW_PITCH - (ARROW_PITCH - ARROW_SIZE)) / 2;
    for(size_t i = 0; i < length; i++) {
        draw_arrow(canvas, x + (int32_t)i * ARROW_PITCH, 16, stratagem_arrow(reinforce, i), true);
    }

    canvas_set_font(canvas, FontSecondary);
    snprintf(
        line,
        sizeof(line),
        "Record %lu (M%lu)",
        (unsigned long)game->best_score,
        (unsigned long)game->best_round);
    draw_center(canvas, 34, line);
    draw_hints(canvas, "OK: jouer", "Back: quitter");
}

static void draw_announce(Canvas* canvas, const Game* game) {
    char line[16];
    canvas_set_font(canvas, FontPrimary);
    draw_center(canvas, 8, "MANCHE");
    canvas_set_font(canvas, FontBigNumbers);
    snprintf(line, sizeof(line), "%lu", (unsigned long)game->round);
    draw_center(canvas, 24, line);
    canvas_set_font(canvas, FontSecondary);
    snprintf(line, sizeof(line), "Score %lu", (unsigned long)game->score);
    canvas_draw_str_aligned(canvas, 64, 63, AlignCenter, AlignBottom, line);
}

#define ROUND_END_REVEAL_MS 500
#define ROUND_END_ROW_Y     16
#define ROUND_END_ROW_STEP  10

static void draw_bonus_row(Canvas* canvas, int32_t row, const char* label, uint32_t value) {
    char number[16];
    int32_t y = ROUND_END_ROW_Y + row * ROUND_END_ROW_STEP;
    snprintf(number, sizeof(number), "+%lu", (unsigned long)value);
    canvas_draw_str_aligned(canvas, 16, y, AlignLeft, AlignTop, label);
    canvas_draw_str_aligned(canvas, 112, y, AlignRight, AlignTop, number);
}

/* Bonuses show one by one, every 500 ms, then the total (REQ-001-12). */
static void draw_round_end(Canvas* canvas, const Game* game) {
    char line[40];
    canvas_set_font(canvas, FontPrimary);
    snprintf(line, sizeof(line), "MANCHE %lu TERMINEE", (unsigned long)game->round);
    draw_center(canvas, 2, line);

    uint32_t elapsed = GAME_ROUND_END_MS - game->phase_ms;
    int32_t shown = (int32_t)(elapsed / ROUND_END_REVEAL_MS) + 1;
    int32_t row = 0;

    canvas_set_font(canvas, FontSecondary);
    if(row < shown) draw_bonus_row(canvas, row++, "Bonus de manche", game->round_bonus);
    if(row < shown) draw_bonus_row(canvas, row++, "Bonus de temps", game->time_bonus);
    if(game->perfect_bonus && row < shown) {
        draw_bonus_row(canvas, row++, "Manche parfaite", game->perfect_bonus);
    }
    if(row < shown) {
        canvas_set_font(canvas, FontPrimary);
        draw_bonus_row(
            canvas, row, "Total", game->round_bonus + game->time_bonus + game->perfect_bonus);
    }

    draw_hints(canvas, "OK: suite", "Back: pause");
}

/* Nothing of the round shows while paused: no reading ahead (REQ-001-16). */
static void draw_paused(Canvas* canvas, const Game* game) {
    char line[40];
    canvas_set_font(canvas, FontPrimary);
    draw_center(canvas, 8, "PAUSE");
    canvas_set_font(canvas, FontSecondary);
    snprintf(
        line,
        sizeof(line),
        "Manche %lu  Score %lu",
        (unsigned long)game->round,
        (unsigned long)game->score);
    draw_center(canvas, 28, line);
    draw_hints(canvas, "OK: reprendre", "Back: quitter");
}

static void draw_game_over(Canvas* canvas, const Game* game) {
    char line[40];
    canvas_set_font(canvas, FontPrimary);
    draw_center(canvas, 2, "GAME OVER");
    canvas_set_font(canvas, FontSecondary);
    snprintf(
        line,
        sizeof(line),
        "Score %lu  Manche %lu",
        (unsigned long)game->score,
        (unsigned long)game->round);
    draw_center(canvas, 16, line);
    snprintf(
        line,
        sizeof(line),
        "Record %lu (M%lu)",
        (unsigned long)game->best_score,
        (unsigned long)game->best_round);
    draw_center(canvas, 27, line);
    if(game->new_record) {
        /* Inverted banner. */
        canvas_draw_box(canvas, 12, 38, 104, 11);
        canvas_set_color(canvas, ColorWhite);
        canvas_set_font(canvas, FontPrimary);
        draw_center(canvas, 39, "NOUVEAU RECORD !");
        canvas_set_color(canvas, ColorBlack);
    }
    draw_hints(canvas, "OK: titre", "Back: quitter");
}

void view_draw(Canvas* canvas, const Game* game) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    switch(game->phase) {
    case GamePhaseTitle:
        draw_title(canvas, game);
        break;
    case GamePhaseAnnounce:
        draw_announce(canvas, game);
        break;
    case GamePhasePlaying:
        draw_playing(canvas, game);
        break;
    case GamePhaseRoundEnd:
        draw_round_end(canvas, game);
        break;
    case GamePhasePaused:
        draw_paused(canvas, game);
        break;
    case GamePhaseGameOver:
        draw_game_over(canvas, game);
        break;
    }
}
