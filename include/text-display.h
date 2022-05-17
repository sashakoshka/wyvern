#pragma once

#include <stdint.h>
#include <cairo.h>
#include "edit-buffer.h"

#define TAB_WIDTH 8

#define TEXTDISPLAY_EMPTY_CELL 1

typedef struct {
        Rune    rune;
        uint8_t damaged;
        uint8_t color;
        uint8_t hasCursor;
        size_t  realRow;
        size_t  realColumn;
} TextDisplay_Cell;

typedef struct {
        EditBuffer *model;

        size_t width;
        size_t height;
        
        TextDisplay_Cell *cells;
} TextDisplay;

TextDisplay *TextDisplay_new           (EditBuffer *, size_t, size_t);
void         TextDisplay_free          (TextDisplay *);
void         TextDisplay_grab          (TextDisplay *);
void         TextDisplay_setModel      (TextDisplay *, EditBuffer *);
void         TextDisplay_resize        (TextDisplay *, size_t, size_t);
void         TextDisplay_getRealCoords (
        TextDisplay *,
        size_t, size_t,
        size_t *, size_t *);
