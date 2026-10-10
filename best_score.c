#include "best_score.h"

#include <flipper_format/flipper_format.h>
#include <furi.h>
#include <storage/storage.h>

#define TAG "FlipperDiverBest"

#define BEST_SCORE_PATH     APP_DATA_PATH("best.fff")
#define BEST_SCORE_FILETYPE "Flipper Diver Best Score"
#define BEST_SCORE_VERSION  1u

bool best_score_load(uint32_t* score, uint32_t* round) {
    *score = 0;
    *round = 0;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* filetype = furi_string_alloc();
    uint32_t version = 0;
    uint32_t file_score = 0;
    uint32_t file_round = 0;

    bool ok = flipper_format_file_open_existing(file, BEST_SCORE_PATH) &&
              flipper_format_read_header(file, filetype, &version) &&
              furi_string_equal_str(filetype, BEST_SCORE_FILETYPE) &&
              version == BEST_SCORE_VERSION &&
              flipper_format_read_uint32(file, "Score", &file_score, 1) &&
              flipper_format_read_uint32(file, "Round", &file_round, 1);
    if(ok) {
        *score = file_score;
        *round = file_round;
    } else {
        /* Expected on first launch: not an error for the player. */
        FURI_LOG_I(TAG, "no valid record in %s", BEST_SCORE_PATH);
    }

    furi_string_free(filetype);
    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
    return ok;
}

bool best_score_save(uint32_t score, uint32_t round) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);

    bool ok = flipper_format_file_open_always(file, BEST_SCORE_PATH) &&
              flipper_format_write_header_cstr(file, BEST_SCORE_FILETYPE, BEST_SCORE_VERSION) &&
              flipper_format_write_uint32(file, "Score", &score, 1) &&
              flipper_format_write_uint32(file, "Round", &round, 1);
    if(!ok) {
        FURI_LOG_E(TAG, "could not write %s", BEST_SCORE_PATH);
    }

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
    return ok;
}
