#pragma once

#include <ctype.h>
#include <cairo.h>

#include "interface.h"
#include "objects.h"
#include "window.h"

// TODO: make this an enum, and make a function to set color based on input from
// a value in that enum.
#define BACKGROUND_COLOR 0.141, 0.161, 0.200
#define OUTLINE_COLOR    0.122, 0.137, 0.169
#define TEXT_COLOR       0.925, 0.937, 0.957
#define ACCENT_COLOR     0.506, 0.631, 0.757

#define RULER_COLOR         0.180, 0.204, 0.251
#define RULER_TEXT_COLOR    0.298, 0.337, 0.416
#define CURSOR_COLOR        0.298, 0.337, 0.416
#define BAD_CHAR_COLOR      0.749, 0.380, 0.419
#define SELECTION_COLOR     0.298, 0.337, 0.416
#define BUTTON_SYMBOL_COLOR 0.682, 0.718, 0.776

#define TAB_BAR_COLOR           0.141, 0.161, 0.200
#define INACTIVE_TAB_COLOR      0.141, 0.161, 0.200
#define ACTIVE_TAB_COLOR        0.188, 0.212, 0.263
#define INACTIVE_TAB_TEXT_COLOR 0.682, 0.718, 0.776
#define ACTIVE_TAB_TEXT_COLOR   0.925, 0.937, 0.957
#define CLOSE_BUTTON_COLOR      0.749, 0.380, 0.419

#define HITBOX(xx, yy, element) \
        xx > (element.x) && xx < (element.x) + (element.width) && \
        yy > (element.y) && yy < (element.y) + (element.height)

#define Interface_Object_invalidateLayout(object) \
        Interface_Object_invalidateLayoutBack((Interface_Object *)(object))

#define Interface_Object_invalidateDrawing(object) \
        Interface_Object_invalidateDrawingBack((Interface_Object *)(object))

extern Interface interface;

void Interface_recalculate               (void);
void Interface_Tab_recalculate           (Interface_Tab *);
void Interface_newTabButton_recalculate  (void);
void Interface_tabBar_recalculate        (void);
void Interface_editView_recalculate      (void);
void Interface_editViewRuler_recalculate (void);
void Interface_editViewText_recalculate  (void);

void Interface_redraw                  (void);
void Interface_Tab_redraw              (Interface_Tab *);
void Interface_newTabButton_redraw     (void);
void Interface_tabBar_redraw           (void);
void Interface_editView_redraw         (void);
void Interface_editViewRuler_redraw    (void);
void Interface_editViewText_redraw     (void);
void Interface_editViewText_redrawRow  (size_t);
void Interface_editViewText_redrawRune (size_t, size_t, int *);

void Interface_refresh               (void);
void Interface_tabBar_refresh        (void);
void Interface_Tab_refresh           (Interface_Tab *);
void Interface_newTabButton_refresh  (void);
void Interface_editView_refresh      (void);
void Interface_editViewRuler_refresh (void);
void Interface_editViewText_refresh  (void);

void Interface_Object_invalidateLayoutBack     (Interface_Object *);
void Interface_Object_invalidateDrawingBack    (Interface_Object *);
void Interface_invalidateLayout                (void);
void Interface_invalidateDrawing               (void);
void Interface_tabBar_invalidateLayout         (void);
void Interface_tabBar_invalidateDrawing        (void);
void Interface_Tab_invalidateLayout            (Interface_Tab *);
void Interface_Tab_invalidateDrawing           (Interface_Tab *);
void Interface_editView_invalidateLayout       (void);
void Interface_editView_invalidateDrawing      (void);

void Interface_handleKeyRight    (Window_State);
void Interface_handleKeyLeft     (Window_State);
void Interface_handleKeyDown     (Window_State);
void Interface_handleKeyUp       (Window_State);
void Interface_handleKey         (int, Window_KeySym, Rune, Window_State);
void Interface_handleInterval    (int);
void Interface_handleMouseMove   (int, int, int);
void Interface_handleMouseButton (int, Window_MouseButton, Window_State);
void Interface_handleRedraw      (int, int, int);

Error Interface_loadFonts      (void);
void  Interface_fontNormal     (void);
// void Interface_fontBold       (void);
// void Interface_fontItalic     (void);
// void Interface_fontBoldItalic (void);

void Interface_findMouseHoverCell  (int, int, size_t *, size_t *);
void Interface_updateTextSelection (void);
