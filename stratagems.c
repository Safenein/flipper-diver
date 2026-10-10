#include "stratagems.h"

#include <string.h>

/* Sequences from Helldivers 2, to be checked by the user (specs/001, D7).
 * Names longer than STRATAGEM_NAME_MAX are shortened.
 * Formatting is off so that the table keeps one entry per line. */
/* clang-format off */
const Stratagem stratagems[STRATAGEM_COUNT] = {
    {"Reinforce",             "UDRLU"},
    {"Resupply",              "DDUR"},
    {"SOS Beacon",            "UDRU"},
    {"Hellbomb",              "DULDURDU"},
    {"Eagle Rearm",           "UULUR"},
    {"Eagle 500kg Bomb",      "URDDD"},
    {"Eagle Airstrike",       "URDR"},
    {"Eagle Strafing Run",    "URR"},
    {"Orbital Laser",         "RDURD"},
    {"Orbital Precision",     "RRU"},
    {"Orbital Railcannon",    "RUDDR"},
    {"Orbital 380mm Barrage", "RDUULDD"},
    {"Machine Gun",           "DLDUR"},
    {"Autocannon",            "DLDUUR"},
    {"Railgun",               "DRDULR"},
    {"Quasar Cannon",         "DDULR"},
    {"Recoilless Rifle",      "DLRRL"},
    {"Shield Generator Pack", "DULRLR"},
    {"Guard Dog",             "DULURD"},
    {"Patriot Exosuit",       "LDRULDD"},
};
/* clang-format on */

size_t stratagem_length(const Stratagem* stratagem) {
    return strlen(stratagem->sequence);
}

StratagemArrow stratagem_arrow(const Stratagem* stratagem, size_t index) {
    switch(stratagem->sequence[index]) {
    case 'U':
        return StratagemArrowUp;
    case 'D':
        return StratagemArrowDown;
    case 'L':
        return StratagemArrowLeft;
    default:
        return StratagemArrowRight;
    }
}
