#include "test_viewport.h"

#include "config.h"
#include "test.h"
#include "viewport.h"
#include "window.h"

static_assert(CHARACTER_RATIO > 0.1f);

static void setup_landscape(void) {
    window.is_init = true;
    window.width = 100;
    window.height = 10;
}

static void setup_portrait(void) {
    window.is_init = true;
    window.width = 10;
    window.height = 100;
}

static void teardown(void) {
    window.is_init = false;
}

START_TEST(test_player_0_with_1_player_window_landscape) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 0, 1);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 100);
    ck_assert_int_eq(viewport.height, 10);
}
END_TEST

START_TEST(test_player_0_with_1_player_window_portrait) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 0, 1);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 10);
    ck_assert_int_eq(viewport.height, 100);
}
END_TEST

START_TEST(test_player_0_with_2_players_window_landscape) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 0, 2);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 50);
    ck_assert_int_eq(viewport.height, 10);
}
END_TEST

START_TEST(test_player_0_with_2_players_window_portrait) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 0, 2);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 10);
    ck_assert_int_eq(viewport.height, 50);
}
END_TEST

START_TEST(test_player_1_with_2_players_window_landscape) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 1, 2);

    ck_assert_int_eq(viewport.x_offset, 51);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 49);
    ck_assert_int_eq(viewport.height, 10);
}
END_TEST

START_TEST(test_player_1_with_2_players_window_portrait) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 1, 2);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 51);
    ck_assert_int_eq(viewport.width, 10);
    ck_assert_int_eq(viewport.height, 49);
}
END_TEST

START_TEST(test_player_0_with_3_players_window_landscape) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 0, 3);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 50);
    ck_assert_int_eq(viewport.height, 10);
}
END_TEST

START_TEST(test_player_0_with_3_players_window_portrait) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 0, 3);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 10);
    ck_assert_int_eq(viewport.height, 50);
}
END_TEST

START_TEST(test_player_1_with_3_players_window_landscape) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 1, 3);

    ck_assert_int_eq(viewport.x_offset, 51);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 49);
    ck_assert_int_eq(viewport.height, 5);
}
END_TEST

START_TEST(test_player_1_with_3_players_window_portrait) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 1, 3);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 51);
    ck_assert_int_eq(viewport.width, 5);
    ck_assert_int_eq(viewport.height, 49);
}
END_TEST

START_TEST(test_player_2_with_3_players_window_landscape) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 2, 3);

    ck_assert_int_eq(viewport.x_offset, 51);
    ck_assert_int_eq(viewport.y_offset, 6);
    ck_assert_int_eq(viewport.width, 49);
    ck_assert_int_eq(viewport.height, 4);
}
END_TEST

START_TEST(test_player_2_with_3_players_window_portrait) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 2, 3);

    ck_assert_int_eq(viewport.x_offset, 6);
    ck_assert_int_eq(viewport.y_offset, 51);
    ck_assert_int_eq(viewport.width, 4);
    ck_assert_int_eq(viewport.height, 49);
}
END_TEST

START_TEST(test_player_0_with_4_players_window_landscape) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 0, 4);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 50);
    ck_assert_int_eq(viewport.height, 5);
}
END_TEST

START_TEST(test_player_0_with_4_players_window_portrait) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 0, 4);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 5);
    ck_assert_int_eq(viewport.height, 50);
}
END_TEST

START_TEST(test_player_1_with_4_players_window_landscape) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 1, 4);

    ck_assert_int_eq(viewport.x_offset, 51);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 49);
    ck_assert_int_eq(viewport.height, 5);
}
END_TEST

START_TEST(test_player_1_with_4_players_window_portrait) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 1, 4);

    ck_assert_int_eq(viewport.x_offset, 6);
    ck_assert_int_eq(viewport.y_offset, 0);
    ck_assert_int_eq(viewport.width, 4);
    ck_assert_int_eq(viewport.height, 50);
}
END_TEST

START_TEST(test_player_2_with_4_players_window_landscape) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 2, 4);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 6);
    ck_assert_int_eq(viewport.width, 50);
    ck_assert_int_eq(viewport.height, 4);
}
END_TEST

START_TEST(test_player_2_with_4_players_window_portrait) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 2, 4);

    ck_assert_int_eq(viewport.x_offset, 0);
    ck_assert_int_eq(viewport.y_offset, 51);
    ck_assert_int_eq(viewport.width, 5);
    ck_assert_int_eq(viewport.height, 49);
}
END_TEST

START_TEST(test_player_3_with_4_players_window_landscape) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 3, 4);

    ck_assert_int_eq(viewport.x_offset, 51);
    ck_assert_int_eq(viewport.y_offset, 6);
    ck_assert_int_eq(viewport.width, 49);
    ck_assert_int_eq(viewport.height, 4);
}
END_TEST

START_TEST(test_player_3_with_4_players_window_portrait) {
    Viewport viewport;
    viewport_from_player_index(&viewport, 3, 4);

    ck_assert_int_eq(viewport.x_offset, 6);
    ck_assert_int_eq(viewport.y_offset, 51);
    ck_assert_int_eq(viewport.width, 4);
    ck_assert_int_eq(viewport.height, 49);
}
END_TEST

// clang-format off
TEST_SUITE(
    viewport,
    TEST_CASE_WITH_SETUP(
        "viewport_from_player_index_window_landscape",
        TEST(test_player_0_with_1_player_window_landscape)
        TEST(test_player_0_with_2_players_window_landscape)
        TEST(test_player_1_with_2_players_window_landscape)
        TEST(test_player_0_with_3_players_window_landscape)
        TEST(test_player_1_with_3_players_window_landscape)
        TEST(test_player_2_with_3_players_window_landscape)
        TEST(test_player_0_with_4_players_window_landscape)
        TEST(test_player_1_with_4_players_window_landscape)
        TEST(test_player_2_with_4_players_window_landscape)
        TEST(test_player_3_with_4_players_window_landscape),
        setup_landscape,
        teardown
    )
    TEST_CASE_WITH_SETUP(
        "viewport_from_player_index_window_portrait",
        TEST(test_player_0_with_1_player_window_portrait)
        TEST(test_player_0_with_2_players_window_portrait)
        TEST(test_player_1_with_2_players_window_portrait)
        TEST(test_player_0_with_3_players_window_portrait)
        TEST(test_player_1_with_3_players_window_portrait)
        TEST(test_player_2_with_3_players_window_portrait)
        TEST(test_player_0_with_4_players_window_portrait)
        TEST(test_player_1_with_4_players_window_portrait)
        TEST(test_player_2_with_4_players_window_portrait)
        TEST(test_player_3_with_4_players_window_portrait),
        setup_portrait,
        teardown
    )
)
// clang-format on
