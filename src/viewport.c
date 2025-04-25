#include "viewport.h"

#include "config.h"
#include "window.h"

void viewport_from_player_index(Viewport *const viewport,
                                const int8_t player_index,
                                const uint8_t number_players) {
    assert(viewport != NULL);
    assert(0 < number_players && number_players <= 4);
    assert(player_index < number_players);
    assert(window.is_init);
    assert(window.width >= 5);
    assert(window.height >= 5);

    if (number_players == 1) {
        assert(player_index == 0);
        viewport->x_offset = 0;
        viewport->y_offset = 0;
        viewport->width = window.width;
        viewport->height = window.height;
        return;
    }

    const bool window_is_landscape =
        CHARACTER_RATIO * window.width / window.height >= 1.0f;
    const int half_width = window.width / 2;
    const int half_height = window.height / 2;

    if (number_players == 2) {
        assert(player_index <= 1);
        if (player_index == 0) {
            viewport->x_offset = 0;
            viewport->y_offset = 0;
            if (window_is_landscape) {
                viewport->width = half_width;
                viewport->height = window.height;
                return;
            }

            viewport->width = window.width;
            viewport->height = half_height;
            return;
        }

        if (window_is_landscape) {
            viewport->x_offset = half_width + 1;
            viewport->y_offset = 0;
            viewport->width = window.width - half_width - 1;
            viewport->height = window.height;
            return;
        }

        viewport->x_offset = 0;
        viewport->y_offset = half_height + 1;
        viewport->width = window.width;
        viewport->height = window.height - half_height - 1;
        return;
    }

    if (number_players == 3) {
        if (player_index == 0) {
            viewport->x_offset = 0;
            viewport->y_offset = 0;
            if (window_is_landscape) {
                viewport->width = half_width;
                viewport->height = window.height;
                return;
            }

            viewport->width = window.width;
            viewport->height = half_height;
            return;
        }

        if (player_index == 1) {
            if (window_is_landscape) {
                viewport->x_offset = half_width + 1;
                viewport->y_offset = 0;
                viewport->width = window.width - half_width - 1;
                viewport->height = half_height;
                return;
            }

            viewport->x_offset = 0;
            viewport->y_offset = half_height + 1;
            viewport->width = half_width;
            viewport->height = window.height - half_height - 1;
            return;
        }

        viewport->x_offset = half_width + 1;
        viewport->y_offset = half_height + 1;
        viewport->width = window.width - half_width - 1;
        viewport->height = window.height - half_height - 1;
        return;
    }

    if (player_index % 2 == 0) {
        viewport->x_offset = 0;
        viewport->width = half_width;
    } else {
        viewport->x_offset = half_width + 1;
        viewport->width = window.width - half_width - 1;
    }

    if (player_index < 2) {
        viewport->y_offset = 0;
        viewport->height = half_height;
    } else {
        viewport->y_offset = half_height + 1;
        viewport->height = window.height - half_height - 1;
    }
}
