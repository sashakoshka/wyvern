#include <ctype.h>
#include <cairo.h>
#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "interface.h"
#include "window.h"
#include "stdio.h"
#include "text-display.h"

#define OUTLINE_COLOR    0.122, 0.137, 0.169
#define TEXT_COLOR       0.925, 0.937, 0.957
#define BACKGROUND_COLOR 0.141, 0.161, 0.200
#define TAB_BAR_COLOR    0.141, 0.161, 0.200
#define RULER_COLOR      0.180, 0.204, 0.251
#define RULER_TEXT_COLOR 0.298, 0.337, 0.416
#define CURSOR_COLOR     0.298, 0.337, 0.416
// #define ACTIVE_TAB_COLOR 0.188, 0.212, 0.263

#define HITBOX(xx, yy, element) \
        xx > element.x && xx < element.x + element.width && \
        yy > element.y && yy < element.y + element.height

static Error Interface_setup                 (void);
static void  Interface_recalculate           (int, int);
static void  Interface_redraw                (void);
static void  Interface_tabBar_redraw         (void);
static void  Interface_editView_redraw       (void);
static void  Interface_editView_drawRuler    (void);
static void  Interface_editView_drawChars    (int);
static void  Interface_editView_drawCharsRow (size_t);

static void fontNormal     (void);
// static void fontBold       (void);
// static void fontItalic     (void);
// static void fontBoldItalic (void);

static void onRedraw      (int, int);
static void onMouseButton (Window_MouseButton, Window_State);
static void onMouseMove   (int, int);
static void onInterval    (void);
static void onKey         (Window_KeySym, Rune, Window_State);

static FT_Library         freetypeHandle     = { 0 };
static FT_Face            freetypeFaceNormal = { 0 };
static cairo_font_face_t *fontFaceNormal     = NULL;
static char              *fontName           =
        "/home/sashakoshka/.local/share/fonts/DMMono-Light.ttf";

static int    fontSize       = 14;
static double glyphHeight    = 0;
static double lineHeight     = 0;
static double glyphWidth     = 0;

static int scrollSize = 8;
static int cursorSize = 2;

Interface interface = { 0 };
static EditBuffer  *editBuffer  = NULL;
static TextDisplay *textDisplay = NULL;

static int mouseX = 0;
static int mouseY = 0;

static struct {
        int shift;
        int lock;
        int ctrl;
        int alt;
} modKeys = { 0 };

Error Interface_run (void) {
        Window_start();
        Error err = Interface_setup();
        if (err) { return err; }
        Window_show();
        
        err = Window_listen();
        Window_stop();

        return err;
}

void Interface_setEditBuffer (EditBuffer *newEditBuffer) {
        editBuffer = newEditBuffer;
}

static Error Interface_setup (void) {
        int err = FT_Init_FreeType(&freetypeHandle);
        if (err) { return Error_cantInitFreetype; }
        err = FT_New_Face(freetypeHandle, fontName, 0, &freetypeFaceNormal);
        if (err) { return Error_cantLoadFont; }
        fontFaceNormal = cairo_ft_font_face_create_for_ft_face (
                freetypeFaceNormal, 0);

        fontNormal();
        cairo_font_extents_t fontExtents;
        cairo_font_extents(Window_context, &fontExtents);
        lineHeight  = fontExtents.height;
        glyphHeight = fontExtents.ascent;
        glyphWidth  = fontExtents.max_x_advance;
        
        if (textDisplay != NULL) { free(textDisplay); }
        textDisplay = TextDisplay_new (
                editBuffer,
                (size_t)interface.width,
                (size_t)interface.height);
        
        Window_onRedraw(onRedraw);
        Window_onMouseButton(onMouseButton);
        Window_onMouseMove(onMouseMove);
        Window_onInterval(onInterval);
        Window_onKey(onKey);
        
        Window_interval = 500;
        Window_setTitle("Text Editor");

        return Error_none;
}

static void Interface_recalculate (int width, int height) {
        interface.width  = width;
        interface.height = height;

        int horizontal = interface.width > interface.height;

        interface.tabBar.x      = 0;
        interface.tabBar.y      = 0;
        interface.tabBar.height = 35;
        interface.tabBar.width  = interface.width;

        Interface_EditView *editView = &interface.editView;
        
        editView->x      = 0;
        editView->y      = interface.tabBar.height;
        editView->height = interface.height - interface.tabBar.height;
        editView->width  = interface.width;
        
        editView->padding    = (int)glyphWidth * 2;
        editView->rulerWidth = (int)(glyphWidth * 5);

        editView->innerX      = editView->x      + editView->padding;
        editView->innerY      = editView->y      + editView->padding;
        editView->innerWidth  = editView->width  - editView->padding;
        editView->innerHeight = editView->height - editView->padding;

        double textLeftOffset = editView->rulerWidth + editView->padding;
        editView->textX = editView->innerX + textLeftOffset;
        editView->textY = editView->innerY + glyphHeight * 0.8;
        editView->textWidth  = editView->width  - textLeftOffset;
        editView->textHeight = editView->height - editView->padding +
                lineHeight;

        double textDisplayWidth  = editView->textWidth  / glyphWidth;
        double textDisplayHeight = editView->textHeight / lineHeight;
        if (textDisplayWidth  < 0) { textDisplayWidth  = 0; }
        if (textDisplayHeight < 0) { textDisplayHeight = 0; }
        TextDisplay_resize (
                textDisplay,
                (size_t)(textDisplayWidth),
                (size_t)(textDisplayHeight));
        
        if (horizontal) {
                
        }
}

static void Interface_redraw (void) {
        Interface_tabBar_redraw();
        Interface_editView_redraw();
}

static void Interface_tabBar_redraw (void) {
        cairo_set_source_rgb(Window_context, TAB_BAR_COLOR);
        cairo_rectangle (
                Window_context,
                interface.tabBar.x,
                interface.tabBar.y,
                interface.tabBar.width,
                interface.tabBar.height);
        cairo_fill(Window_context);
                
        cairo_set_source_rgb(Window_context, OUTLINE_COLOR);
        cairo_set_line_width(Window_context, 1);
        cairo_move_to (
                Window_context,
                interface.tabBar.x,
                interface.tabBar.y + interface.tabBar.height - 0.5);
        cairo_line_to (
                Window_context,
                interface.tabBar.x + interface.tabBar.width,
                interface.tabBar.y + interface.tabBar.height - 0.5);
        cairo_stroke(Window_context);
}

static void Interface_editView_redraw (void) {
        Interface_EditView *editView = &interface.editView;
        
        cairo_set_source_rgb(Window_context, BACKGROUND_COLOR);
        cairo_rectangle (
                Window_context,
                editView->x,
                editView->y,
                editView->width,
                editView->height);
        cairo_fill(Window_context);

        cairo_set_source_rgb(Window_context, RULER_COLOR);
        cairo_set_line_width(Window_context, 2);
        double columnMarkerX = editView->textX + glyphWidth * 80 + 1;
        cairo_move_to(Window_context, columnMarkerX, editView->y);
        cairo_line_to (
                Window_context,
                columnMarkerX,
                editView->y + editView->height);
        cairo_stroke(Window_context);

        Interface_editView_drawRuler();
        Interface_editView_drawChars(1);
}

static void Interface_editView_drawRuler (void) {
        Interface_EditView *editView = &interface.editView;
        
        cairo_set_source_rgb(Window_context, RULER_COLOR);
        cairo_rectangle (
                Window_context,
                editView->x,
                editView->y,
                editView->innerX + editView->rulerWidth,
                editView->height);
        cairo_fill(Window_context);
        
        double y = editView->textY;
        for (
                size_t index = editBuffer->scroll;
                index < editBuffer->length &&
                y < editView->textY + editView->textHeight;
                index ++
        ) {     
                char lineNumberBuffer[8] = { 0 };
                snprintf(lineNumberBuffer, 7, "%zu", index + 1);
                
                cairo_set_source_rgb(Window_context, RULER_TEXT_COLOR);
                cairo_move_to (
                        Window_context,
                        editView->innerX, y);
                cairo_show_text(Window_context, lineNumberBuffer);

                y += lineHeight;
        }
}

static void Interface_editView_drawChars (int grabModel) {
        if (grabModel) { TextDisplay_grab(textDisplay); }

        for (size_t y = 0; y < textDisplay->height; y ++) {
                Interface_editView_drawCharsRow(y);
        }
}

static void Interface_editView_drawCharsRow (size_t y) {
        Interface_EditView *editView = &interface.editView;
        int inIndent = 1;
        
        for (size_t x = 0; x < textDisplay->width; x ++) {
                size_t coordinate = y * textDisplay->width + x;
                TextDisplay_Cell *cell = &textDisplay->cells[coordinate];
                
                if (!cell->damaged && !cell->hasCursor) { continue; }
                textDisplay->cells[coordinate].damaged = 0;
                
                double realX = editView->textX;
                double realY = editView->innerY;
                realX += (double)(x) * glyphWidth;
                realY += (double)(y) * lineHeight;
                
                cairo_set_source_rgb(Window_context, BACKGROUND_COLOR);
                cairo_rectangle (
                        Window_context,
                        realX, realY,
                        glyphWidth, lineHeight);
                cairo_fill(Window_context);

                int isSpace = isspace((char)(cell->rune));
                if (!isSpace) { inIndent = 0; }
                
                if (x % TAB_WIDTH == 0 && inIndent) {
                        cairo_set_source_rgb(Window_context, RULER_COLOR);
                        cairo_set_line_width(Window_context, 2);
                        cairo_move_to(Window_context, realX + 1, realY);
                        cairo_line_to (
                                Window_context,
                                realX + 1,
                                realY + glyphHeight);
                        cairo_stroke(Window_context);
                }
                
                if (cell->hasCursor && editView->cursorBlink) {
                        cairo_set_source_rgb(Window_context, CURSOR_COLOR);
                        cairo_set_line_width(Window_context, cursorSize);
                        cairo_move_to (
                                Window_context,
                                realX + cursorSize / 2,
                                realY);
                        cairo_line_to (
                                Window_context,
                                realX + cursorSize / 2,
                                realY + glyphHeight);
                        cairo_stroke(Window_context);
                }

                if (x == 80) {
                        cairo_set_source_rgb(Window_context, RULER_COLOR);
                        cairo_set_line_width(Window_context, 2);
                        cairo_move_to(Window_context, realX + 1, realY);
                        cairo_line_to (
                                Window_context,
                                realX + 1,
                                realY + lineHeight);
                        cairo_stroke(Window_context);
                }

                if (cell->rune == 0 || isSpace) { continue; }
                cairo_glyph_t glyph = {
                        .index = FT_Get_Char_Index (
                                freetypeFaceNormal,
                                cell->rune),
                        .x     = realX,
                        .y     = realY + glyphHeight * 0.8
                };
                fontNormal();
                cairo_set_source_rgb(Window_context, TEXT_COLOR);
                cairo_show_glyphs(Window_context, &glyph, 1);
        }
}

static void fontNormal (void) {
        cairo_set_font_size(Window_context, fontSize);
        cairo_set_font_face(Window_context, fontFaceNormal);
}

// static void fontBold (void) {
        // cairo_set_font_size(Window_context, fontSize);
        // cairo_set_font_face(Window_context, fontFaceBold);
// }

// static void fontItalic (void) {
        // cairo_set_font_size(Window_context, fontSize);
        // cairo_set_font_face(Window_context, fontFaceItalic);
// }

// static void fontBoldItalic (void) {
        // cairo_set_font_size(Window_context, fontSize);
        // cairo_set_font_face(Window_context, fontFaceBoldItalic);
// }

static void onRedraw (int width, int height) {
        Interface_recalculate(width, height);
        Interface_redraw();
}

static void onMouseButton (Window_MouseButton button, Window_State state) {
        // something is going to move or change - we need the cursor to be
        // visible
        interface.editView.cursorBlink = 1;

        int inCell = HITBOX(mouseX, mouseY, interface.editView);

        size_t cellX = 0;
        size_t cellY = 0;
        if (inCell) {
                int intCellX = (int) (
                        (mouseX - interface.editView.textX) /
                        glyphWidth);
                int intCellY = (int) (
                        (mouseY - interface.editView.innerY) /
                        lineHeight);

                cellX = (size_t)(intCellX);
                cellY = (size_t)(intCellY);
                
                if (intCellX < 0) { cellX = 0; }
                if (intCellY < 0) { cellY = 0; }
                
                if (cellX >= textDisplay->width) {
                        cellX = textDisplay->width - 1;
                }
                if (cellY >= textDisplay->height) {
                        cellY = textDisplay->height - 1;
                }
        }

        switch (button) {
        case Window_MouseButton_left:
                // TODO: selection, etc.
                if (state == Window_State_on && inCell) {
                        size_t realX = 0;
                        size_t realY = 0;
                        TextDisplay_getRealCoords (
                                textDisplay,
                                cellX, cellY,
                                &realX, &realY);
                        EditBuffer_cursorMoveTo(editBuffer, realX, realY);
                        Interface_editView_drawChars(1);
                }
                break;
        case Window_MouseButton_middle:
                // TODO: copy/paste
                break;
        case Window_MouseButton_right:
                // TODO: context menu
                break;
        case Window_MouseButton_scrollUp:
                if (state == Window_State_on && inCell) {
                        EditBuffer_scroll(editBuffer, scrollSize * -1);
                        Interface_editView_drawRuler();
                        Interface_editView_drawChars(1);
                }
                break;
                
        case Window_MouseButton_scrollDown:
                if (state == Window_State_on && inCell) {
                        EditBuffer_scroll(editBuffer, scrollSize);
                        Interface_editView_drawRuler();
                        Interface_editView_drawChars(1);
                }
                break;
        }
}

static void onMouseMove (int x, int y) {
        mouseX = x;
        mouseY = y;
}

static void onInterval (void) {
        Interface_editView_drawChars(0);
        interface.editView.cursorBlink = !interface.editView.cursorBlink;
}

static void onKey (Window_KeySym keySym, Rune rune, Window_State state) {
        (void)(keySym);
        (void)(rune);
        (void)(state);
}
 
