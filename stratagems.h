/* Stratagem table: names and arrow sequences. Pure C, no Furi (ADR-0004);
 * the matching icons live in view.c, in the same order. */
#pragma once

#include <stddef.h>
#include <stdint.h>

#define STRATAGEM_COUNT        20
#define STRATAGEM_SEQUENCE_MIN 3
#define STRATAGEM_SEQUENCE_MAX 8
/* Longest name that fits on one line in FontSecondary (~6 px per char). */
#define STRATAGEM_NAME_MAX     21

typedef enum {
    StratagemArrowUp,
    StratagemArrowDown,
    StratagemArrowLeft,
    StratagemArrowRight,
} StratagemArrow;

typedef struct {
    const char* name;
    /* One letter per arrow: U, D, L or R. */
    const char* sequence;
} Stratagem;

extern const Stratagem stratagems[STRATAGEM_COUNT];

/* Number of arrows in the stratagem's sequence. */
size_t stratagem_length(const Stratagem* stratagem);

/* Arrow at position index; index must be below stratagem_length(). */
StratagemArrow stratagem_arrow(const Stratagem* stratagem, size_t index);
