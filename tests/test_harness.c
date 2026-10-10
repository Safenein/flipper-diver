#include "test.h"

static void test_check_passes_on_true(void) {
    CHECK(1 + 1 == 2);
    CHECK_EQ(3 * 4, 12);
}

int main(void) {
    RUN_TEST(test_check_passes_on_true);
    return test_report();
}
