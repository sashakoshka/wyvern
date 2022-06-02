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

#define RULER_COLOR               0.180, 0.204, 0.251
#define RULER_TEXT_COLOR          0.298, 0.337, 0.416
#define CURSOR_COLOR              0.298, 0.337, 0.416
#define BAD_CHAR_COLOR            0.749, 0.380, 0.419
#define SELECTION_COLOR           0.298, 0.337, 0.416
#define BUTTON_SYMBOL_COLOR       0.682, 0.718, 0.776
#define BUTTON_SYMBOL_HOVER_COLOR 0.298, 0.337, 0.416
#define BUTTON_SYMBOL_CLICK_COLOR 0.188, 0.212, 0.263

#define TAB_BAR_COLOR            0.141, 0.161, 0.200
#define HOVER_TAB_COLOR          0.188, 0.212, 0.263
#define INACTIVE_TAB_COLOR       0.141, 0.161, 0.200
#define ACTIVE_TAB_COLOR         0.188, 0.212, 0.263
#define INACTIVE_TAB_TEXT_COLOR  0.682, 0.718, 0.776
#define ACTIVE_TAB_TEXT_COLOR    0.925, 0.937, 0.957
#define CLOSE_BUTTON_HOVER_COLOR 0.749, 0.380, 0.419
#define CLOSE_BUTTON_CLICK_COLOR 0.659, 0.333, 0.365

#define TO_GENERIC(object) (Interface_Object *)(object)

#define Interface_Object_isWithinBounds(object, x, y) \
        Interface_Object_isWithinBoundsBack(TO_GENERIC(object), x, y)

#define Interface_Object_invalidateLayout(object) \
        Interface_Object_invalidateLayoutBack(TO_GENERIC(object))

#define Interface_Object_invalidateDrawing(object) \
        Interface_Object_invalidateDrawingBack(TO_GENERIC(object))

#define Interface_Object_isHovered(object) \
        interface.mouseState.hoverObject == TO_GENERIC(object)
        
#define Interface_Object_isNewlyHovered(object) ( \
        interface.mouseState.hoverObject == TO_GENERIC(object) && \
        interface.mouseState.previousHoverObject != TO_GENERIC(object))

#define Interface_Object_isClicked(object) \
        interface.mouseState.downObject == TO_GENERIC(object)

extern Interface interface;

Interface_Object *Interface_getHoveredObject          (int, int);
Interface_Object *Interface_tabBar_getHoveredObject   (int, int);
Interface_Object *Interface_editView_getHoveredObject (int, int);
Interface_Object *Interface_Tab_getHoveredObject (
        Interface_Tab *,
        int, int);

int Interface_Object_isWithinBoundsBack (Interface_Object *, int, int);

void Interface_recalculate                (void);
void Interface_tabBar_recalculate         (void);
void Interface_Tab_recalculate            (Interface_Tab *);
void Interface_newTabButton_recalculate   (void);
void Interface_editView_recalculate       (void);
void Interface_editViewRuler_recalculate  (void);
void Interface_editViewText_recalculate   (void);

void Interface_tabBar_scroll (int);

void Interface_redraw                  (void);
void Interface_tabBar_redraw           (void);
void Interface_Tab_redraw              (Interface_Tab *);
void Interface_TabCloseButton_redraw   (Interface_TabCloseButton *);
void Interface_newTabButton_redraw     (void);
void Interface_editView_redraw         (void);
void Interface_editViewRuler_redraw    (void);
void Interface_editViewText_redraw     (void);
void Interface_editViewText_redrawRow  (size_t);
void Interface_editViewText_redrawRune (size_t, size_t, int *);

void Interface_refresh                (void);
void Interface_tabBar_refresh         (void);
void Interface_Tab_refresh            (Interface_Tab *);
void Interface_TabCloseButton_refresh (Interface_TabCloseButton *);
void Interface_newTabButton_refresh   (void);
void Interface_editView_refresh       (void);
void Interface_editViewRuler_refresh  (void);
void Interface_editViewText_refresh   (void);

void Interface_Object_invalidateLayoutBack  (Interface_Object *);
void Interface_Object_invalidateDrawingBack (Interface_Object *);
void Interface_invalidateLayout             (void);
void Interface_invalidateDrawing            (void);
void Interface_tabBar_invalidateLayout      (void);
void Interface_tabBar_invalidateDrawing     (void);
void Interface_Tab_invalidateLayout         (Interface_Tab *);
void Interface_Tab_invalidateDrawing        (Interface_Tab *);
void Interface_editView_invalidateLayout    (void);
void Interface_editView_invalidateDrawing   (void);

void Interface_editViewText_invalidateText (void);

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

void Interface_roundedRectangle (double, double, double, double, double);

Interface_Tab *Interface_Tab_new  (void);
void           Interface_Tab_free (Interface_Tab *);
