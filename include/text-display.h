#pragma once

#include <stdint.h>
#include <cairo.h>
#include "edit-buffer.h"

typedef struct {
        EditBuffer *model;

        size_t width;
        size_t height;
        
        Rune    *buffer;
        uint8_t *damageBuffer;
        uint8_t *colorBuffer;
        // TODO: implement real coordinate buffer to map display charachters to
        // actual characters in the model, and update them when grabbing. This
        // will make mouse input easier.
} TextDisplay;

TextDisplay *TextDisplay_new      (EditBuffer *, size_t, size_t);
void         TextDisplay_free     (TextDisplay *);
void         TextDisplay_grab     (TextDisplay *);
void         TextDisplay_setModel (TextDisplay *, EditBuffer *);
void         TextDisplay_resize   (TextDisplay *, size_t, size_t);

