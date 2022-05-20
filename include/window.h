#pragma once
#include <cairo.h>
#include <stdlib.h>
#include "error.h"
#include "unicode.h"

#define WINDOW_KEY_ESCAPE      0xFF1B
#define WINDOW_KEY_PAUSE_BREAK 0xFF13
#define WINDOW_KEY_MENU        0xFF67

#define WINDOW_KEY_ENTER     0xFF0D
#define WINDOW_KEY_BACKSPACE 0xFF08
#define WINDOW_KEY_TAB       0xFF09

#define WINDOW_KEY_UP       0xFF52
#define WINDOW_KEY_DOWN     0xFF54
#define WINDOW_KEY_LEFT     0xFF51
#define WINDOW_KEY_RIGHT    0xFF53
#define WINDOW_KEY_HOME     0xFF50
#define WINDOW_KEY_END      0xFF57
#define WINDOW_KEY_PAGEUP   0xFF55
#define WINDOW_KEY_PAGEDOWN 0xFF56

#define WINDOW_KEY_PAD_ENTER 0xFF8D
#define WINDOW_KEY_PAD_0     0xFF9E
#define WINDOW_KEY_PAD_1     0xFF9C
#define WINDOW_KEY_PAD_2     0xFF99
#define WINDOW_KEY_PAD_3     0xFF9B
#define WINDOW_KEY_PAD_4     0xFF96
#define WINDOW_KEY_PAD_5     0xFF9D
#define WINDOW_KEY_PAD_6     0xFF98
#define WINDOW_KEY_PAD_7     0xFF95
#define WINDOW_KEY_PAD_8     0xFF97
#define WINDOW_KEY_PAD_9     0xFF9A
#define WINDOW_KEY_PAD_STAR  0xFFAA
#define WINDOW_KEY_PAD_PLUS  0xFFAB
#define WINDOW_KEY_PAD_MINUS 0xFFAD
#define WINDOW_KEY_PAD_SLASH 0xFFAF

#define WINDOW_KEY_SHIFT 0xFFE1
#define WINDOW_KEY_CTRL  0xFFE3
#define WINDOW_KEY_ALT   0xFFE9
#define WINDOW_KEY_SUPER 0xFFEB

#define WINDOW_KEY_CAPS_LOCK   0xFFE5
#define WINDOW_KEY_NUM_LOCK    0xFF7F
#define WINDOW_KEY_SCROLL_LOCK 0xFF14
#define WINDOW_KEY_INSERT      0xFF63

#define WINDOW_KEY_F1  0xFFBE
#define WINDOW_KEY_F2  0xFFBF
#define WINDOW_KEY_F3  0xFFC0
#define WINDOW_KEY_F4  0xFFC1
#define WINDOW_KEY_F5  0xFFC2
#define WINDOW_KEY_F6  0xFFC3
#define WINDOW_KEY_F7  0xFFC4
#define WINDOW_KEY_F8  0xFFC5
#define WINDOW_KEY_F9  0xFFC6
#define WINDOW_KEY_F10 0xFFC7
#define WINDOW_KEY_F11 0xFFC8
#define WINDOW_KEY_F12 0xFFC9

#define WINDOW_KEY_DELETE 0xFFFF

typedef enum {
        Window_MouseButton_left,
        Window_MouseButton_middle,
        Window_MouseButton_right,
        Window_MouseButton_scrollUp,
        Window_MouseButton_scrollDown,
} Window_MouseButton;

typedef enum {
        Window_State_on  = 1,
        Window_State_off = 0,
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
