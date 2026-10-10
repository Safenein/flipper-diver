#include "test.h"

#include <string.h>

#include "stratagems.h"

static void test_every_entry_is_filled(void) {
    for(size_t i = 0; i < STRATAGEM_COUNT; i++) {
        CHECK(stratagems[i].name != NULL);
        CHECK(stratagems[i].sequence != NULL);
    }
}

static void test_sequences_have_3_to_8_valid_arrows(void) {
    for(size_t i = 0; i < STRATAGEM_COUNT; i++) {
        const Stratagem* s = &stratagems[i];
        size_t length = stratagem_length(s);
        CHECK(length >= STRATAGEM_SEQUENCE_MIN);
        CHECK(length <= STRATAGEM_SEQUENCE_MAX);
        CHECK(strspn(s->sequence, "UDLR") == length);
    }
}

static void test_names_are_unique_and_fit_on_one_line(void) {
    for(size_t i = 0; i < STRATAGEM_COUNT; i++) {
        CHECK(strlen(stratagems[i].name) > 0);
        CHECK(strlen(stratagems[i].name) <= STRATAGEM_NAME_MAX);
        for(size_t j = i + 1; j < STRATAGEM_COUNT; j++) {
            CHECK(strcmp(stratagems[i].name, stratagems[j].name) != 0);
        }
    }
}

static void test_arrow_decoding(void) {
    const Stratagem s = {"Test", "UDLR"};
    CHECK_EQ(stratagem_arrow(&s, 0), StratagemArrowUp);
    CHECK_EQ(stratagem_arrow(&s, 1), StratagemArrowDown);
    CHECK_EQ(stratagem_arrow(&s, 2), StratagemArrowLeft);
    CHECK_EQ(stratagem_arrow(&s, 3), StratagemArrowRight);
}

int main(void) {
    RUN_TEST(test_every_entry_is_filled);
    RUN_TEST(test_sequences_have_3_to_8_valid_arrows);
    RUN_TEST(test_names_are_unique_and_fit_on_one_line);
    RUN_TEST(test_arrow_decoding);
    return test_report();
}
