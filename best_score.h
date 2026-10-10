/* Best score file on the SD card: APP_DATA_PATH("best.fff"), FlipperFormat,
 * Version 1, keys Score and Round (REQ-001-20, 21). */
#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Reads the record. On any failure (no SD card, missing or corrupt file,
 * unknown version) sets both values to 0 and returns false. */
bool best_score_load(uint32_t* score, uint32_t* round);

/* Writes the record, replacing the file. Logs and returns false on failure;
 * the game goes on either way. */
bool best_score_save(uint32_t score, uint32_t round);
