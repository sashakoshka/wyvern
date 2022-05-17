#pragma once
#include <cairo.h>
#include <stdlib.h>
#include "error.h"
#include "unicode.h"

typedef enum {
        Window_MouseButton_left,
        Window_MouseButton_middle,
        Window_MouseButton_right,
        Window_MouseButton_scrollUp,
        Window_MouseButton_scrollDown,
} Window_MouseButton;

typedef enum {
        Window_State_on,
        Window_State_off,
} Window_State;

typedef unsigned long Window_KeySym;

extern cairo_surface_t *Window_surface;
extern cairo_t         *Window_context;
extern time_t           Window_interval;

Error Window_start    (void);
Error Window_show     (void);
Error Window_listen   (void);
Error Window_stop     (void);
Error Window_setTitle (const char *);

void Window_onRedraw      (void (*) (int, int));
void Window_onMouseButton (void (*) (Window_MouseButton, Window_State));
void Window_onMouseMove   (void (*) (int, int));
void Window_onInterval    (void (*) (void));
void Window_onKey         (void (*) (Window_KeySym, Rune, Window_State));
