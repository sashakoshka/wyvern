#pragma once

#include <stdint.h>
#include <cairo.h>
#include "edit-buffer.h"

#define TEXTDISPLAY_EMPTY_CELL 1

// TODO: make one for the currently active selected line(s) and the main cursor
typedef enum {
        TextDisplay_CursorState_none      = 0,
        TextDisplay_CursorState_cursor    = 1,
        TextDisplay_CursorState_selection = 2
} TextDisplay_CursorState;

typedef struct {
        TextDisplay_CursorState cursorState;
        Rune                    rune;
        uint8_t                 damaged;
        uint8_t                 color;
        size_t                  realRow;
        size_t                  realColumn;
} TextDisplay_Cell;

typedef struct {
        EditBuffer *model;

        size_t width;
        size_t height;

        size_t lastRow;
        size_t lastRealRow;
        size_t lastRealColumn;
        
        TextDisplay_Cell *cells;
} TextDisplay;

TextDisplay *TextDisplay_new           (EditBuffer *, size_t, size_t);
void         TextDisplay_free          (TextDisplay *);
void         TextDisplay_grab          (TextDisplay *);
void         TextDisplay_setModel      (TextDisplay *, EditBuffer *);
void         TextDisplay_resize        (TextDisplay *, size_t, size_t);
int          TextDisplay_frameCursors  (TextDisplay *);
void         TextDisplay_getRealCoords (
        TextDisplay *,
        size_t, size_t,
        size_t *, size_t *);
