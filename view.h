/* Rendering of every game phase on the 128x64 screen. */
#pragma once

#include <gui/canvas.h>

#include "game.h"

/* Draws the game. Called from the GUI thread with the game lock held; only
 * reads the game. */
void view_draw(Canvas* canvas, const Game* game);
