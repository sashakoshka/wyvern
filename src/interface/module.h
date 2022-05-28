#include <ctype.h>
#include <cairo.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "interface.h"
#include "window.h"
#include "text-display.h"

#define OUTLINE_COLOR      0.122, 0.137, 0.169
#define TEXT_COLOR         0.925, 0.937, 0.957
#define BACKGROUND_COLOR   0.141, 0.161, 0.200
#define TAB_BAR_COLOR      0.141, 0.161, 0.200
#define RULER_COLOR        0.180, 0.204, 0.251
#define RULER_TEXT_COLOR   0.298, 0.337, 0.416
#define CURSOR_COLOR       0.298, 0.337, 0.416
#define BAD_CHAR_COLOR     0.749, 0.380, 0.419
#define SELECTION_COLOR    0.298, 0.337, 0.416
#define INACTIVE_TAB_COLOR 0.180, 0.204, 0.251
#define ACTIVE_TAB_COLOR   0.188, 0.212, 0.263
#define CLOSE_BUTTON_COLOR 0.749, 0.380, 0.419

#define HITBOX(xx, yy, element) \
        xx > (element.x) && xx < (element.x) + (element.width) && \
        yy > (element.y) && yy < (element.y) + (element.height)

struct Interface_Tab {
        double x;
        double y;
        double width;
        double height;

        double textX;
        double textY;

        char text[NAME_MAX + 1];

        double closeX;
        double closeY;
        double closeWidth;
        double closeHeight;

        struct Interface_Tab *previous;
        struct Interface_Tab *next;
};

typedef struct {
        double x;
        double y;
        double width;
        double height;

        Interface_Tab *tabs;
        
} Interface_TabBar;

typedef struct {
        double x;
        double y;
        double width;
        double height;
        
        double padding;
        double rulerWidth;

        double innerX;
        double innerY;
        double innerWidth;
        double innerHeight;

        double textX;
        double textY;
        double textWidth;
        double textHeight;

        int cursorBlink;
} Interface_EditView;

typedef struct {
        int width;
        int height;
        int horizontal;

        Interface_TabBar   tabBar;
        Interface_EditView editView;
} Interface;

typedef struct {
        int x;
        int y;

        Window_State left;
        Window_State middle;
        Window_State right;

        int    dragOriginX;
        int    dragOriginY;
        int    dragOriginInCell;
        size_t dragOriginRealX;
        size_t dragOriginRealY;
} Interface_MouseState;

typedef struct {
        Window_State shift;
        Window_State ctrl;
        Window_State alt;
} Interface_ModKeyState;

extern Interface    interface;
extern EditBuffer  *editBuffer;
extern TextDisplay *textDisplay;

extern Interface_MouseState  mouseState;
extern Interface_ModKeyState modKeyState;

extern FT_Library         freetypeHandle;
extern FT_Face            freetypeFaceNormal;
extern cairo_font_face_t *fontFaceNormal;

extern double glyphHeight;
extern double lineHeight;
extern double glyphWidth;
extern double capitalHeight;

void Interface_recalculate          (int, int);
void Interface_tabBar_recalculate   (void);
void Interface_editView_recalculate (void);

void Interface_redraw                (void);
void Interface_tabBar_redraw         (void);
void Interface_Tab_redraw            (Interface_Tab *tab);
void Interface_editView_redraw       (void);
void Interface_editView_drawRuler    (void);
void Interface_editView_drawChars    (int);
void Interface_editView_drawCharsRow (size_t);

void Interface_onKeyRight    (Window_State);
void Interface_onKeyLeft     (Window_State);
void Interface_onKeyDown     (Window_State);
void Interface_onKeyUp       (Window_State);
void Interface_onKey         (Window_KeySym, Rune, Window_State);
void Interface_onInterval    (void);
void Interface_onMouseMove   (int, int);
void Interface_onMouseButton (Window_MouseButton, Window_State);
void Interface_onRedraw      (int, int);

Error Interface_loadFonts      (void);
void  Interface_fontNormal     (void);
// void Interface_fontBold       (void);
// void Interface_fontItalic     (void);
// void Interface_fontBoldItalic (void);

void Interface_findMouseHoverCell  (int, int, size_t *, size_t *);
void Interface_updateTextSelection (void);
