#include "module.h"
#include "options.h"

/* Interface_editViewText_recalculate
 * Recalculates the size and position of the text.
 */
void Interface_editViewText_recalculate (void) {
        Interface_EditView      *editView = &interface.editView;
        Interface_EditViewText  *text     = &editView->text;

        double textLeftOffset = editView->ruler.width + editView->padding;
        text->x = editView->innerX + textLeftOffset;
        text->y = editView->innerY + interface.fonts.glyphHeight * 0.8;
        text->width  = editView->width  - textLeftOffset;
        text->height = editView->height - editView->padding +
                interface.fonts.lineHeight;

        double textDisplayWidth =
                text->width  / interface.fonts.glyphWidth;
        double textDisplayHeight =
                text->height / interface.fonts.lineHeight;
        if (textDisplayWidth  < 0) { textDisplayWidth  = 0; }
        if (textDisplayHeight < 0) { textDisplayHeight = 0; }
        TextDisplay_resize (
                text->display,
                (size_t)(textDisplayWidth),
                (size_t)(textDisplayHeight));
}

/* Interface_editViewText_redraw
 * Updates the internal TextDisplay and redraws damaged cells.
 */
void Interface_editViewText_redraw (void) {
        Interface_EditView     *editView = &interface.editView;
        Interface_EditViewText *text     = &editView->text;

        for (size_t y = 0; y < text->display->height; y ++) {
                Interface_editViewText_redrawRow(y);
        }
}

/* Interface_editViewText_redrawRow
 * Redraws damaged cells at row y.
 */
void Interface_editViewText_redrawRow (size_t y) {
        Interface_EditView     *editView = &interface.editView;
        Interface_EditViewText *text     = &editView->text;
        int inIndent = 1;
        
        for (size_t x = 0; x < text->display->width; x ++) {
                Interface_editViewText_redrawRune(x, y, &inIndent);
        }
}

/* Interface_editViewText_redrawRow
 * Redraws the cell at column x and row y if it is damaged.
 */
void Interface_editViewText_redrawRune (size_t x, size_t y, int *inIndent) {
        Interface_EditView     *editView = &interface.editView;
        Interface_EditViewText *text     = &editView->text;
        
        size_t coordinate = y * text->display->width + x;
        TextDisplay_Cell *cell =
                &text->display->cells[coordinate];

        // if the cell is undamaged, we don't want to render it.
        // however, we'll make an exception for cursors because those
        // need to blink.
        if (
                !cell->damaged &&
                cell->cursorState != TextDisplay_CursorState_cursor
        ) { return; }
        
        text->display->cells[coordinate].damaged = 0;
        
        double realX = text->x;
        double realY = editView->innerY;
        realX += (double)(x) * interface.fonts.glyphWidth;
        realY += (double)(y) * interface.fonts.lineHeight;

        // background to clear what was previously there
        cairo_set_source_rgb(Window_context, BACKGROUND_COLOR);
        cairo_rectangle (
                Window_context,
                realX, realY,
                interface.fonts.glyphWidth, interface.fonts.lineHeight);
        cairo_fill(Window_context);

        // draw indentation markers every tab stop
        int isSpace = isspace((char)(cell->rune));
        if (!isSpace) { inIndent = 0; }                
        if (x % (size_t)(Options_tabSize) == 0 && inIndent) {
                cairo_set_source_rgb(Window_context, RULER_COLOR);
                cairo_set_line_width(Window_context, 2);
                cairo_move_to(Window_context, realX + 1, realY);
                cairo_line_to (
                        Window_context,
                        realX + 1,
                        realY + interface.fonts.glyphHeight);
                cairo_stroke(Window_context);
        }

        // draw 80 column marker
        if (x == Options_columnGuide) {
                cairo_set_source_rgb(Window_context, RULER_COLOR);
                cairo_set_line_width(Window_context, 2);
                cairo_move_to(Window_context, realX + 1, realY);
                cairo_line_to (
                        Window_context,
                        realX + 1,
                        realY + interface.fonts.lineHeight);
                cairo_stroke(Window_context);
        }

        // selection highlight
        if (cell->cursorState == TextDisplay_CursorState_selection) {
                cairo_set_source_rgb(Window_context, SELECTION_COLOR);
                cairo_rectangle (
                        Window_context,
                        realX, realY,
                        interface.fonts.glyphWidth,
                        interface.fonts.glyphHeight);
                cairo_fill(Window_context);
        }

        // don't attempt to render whitespace
        if (cell->rune != TEXTDISPLAY_EMPTY_CELL && !isSpace) {
                unsigned int index = FT_Get_Char_Index (
                        interface.fonts.freetypeFaceNormal,
                        cell->rune);

                // if we couldn't find the character, display a red
                // error symbol
                if (index == 0) {
                        cairo_set_source_rgb (
                                Window_context,
                                BAD_CHAR_COLOR);
                        double scale =
                                interface.fonts.glyphWidth / 3;
                        double centerX =
                                realX + interface.fonts.glyphWidth / 2;
                        double centerY =
                                realY + interface.fonts.glyphHeight / 2;
                        cairo_set_line_width(Window_context, 2);
                        cairo_move_to (
                                Window_context,
                                centerX - scale,
                                centerY - scale);
                        cairo_line_to (
                                Window_context,
                                centerX + scale,
                                centerY + scale);
                                cairo_stroke(Window_context);
                        cairo_move_to (
                                Window_context,
                                centerX + scale,
                                centerY - scale);
                        cairo_line_to (
                                Window_context,
                                centerX - scale,
                                centerY + scale);
                                cairo_stroke(Window_context);
                        return;
                }
                
                cairo_glyph_t glyph = {
                        .index = index,
                        .x     = realX,
                        .y     =
                                realY +
                                interface.fonts.glyphHeight * 0.8
                };
                Interface_fontNormal();
                cairo_set_source_rgb(Window_context, TEXT_COLOR);
                cairo_show_glyphs(Window_context, &glyph, 1);
        }
        
        // draw blinking cursor yayayayayayaya
        if (
                cell->cursorState == TextDisplay_CursorState_cursor &&
                text->cursorBlink
        ) {
                cairo_set_source_rgb(Window_context, CURSOR_COLOR);
                cairo_set_line_width (
                        Window_context,
                        Options_cursorSize);
                cairo_move_to (
                        Window_context,
                        realX + (double)(Options_cursorSize) / 2,
                        realY);
                cairo_line_to (
                        Window_context,
                        realX + (double)(Options_cursorSize) / 2,
                        realY + interface.fonts.glyphHeight);
                cairo_stroke(Window_context);
        }
}

/* Interface_editViewText_invalidateText
 * Invalidates the text content of the edit view.
 */
void Interface_editViewText_invalidateText (void) {
        Interface_EditView *editView = &interface.editView;
        editView->text.needsGrab = 1;
}

/* Interface_editViewText_refresh
 * Refreshes the text.
 */
void Interface_editViewText_refresh (void) {
        Interface_EditViewText *text = &interface.editView.text;
        
        if (text->needsRecalculate == 1) {
                Interface_editViewText_recalculate();
                text->needsRecalculate = 0;
        }

        if (text->needsGrab == 1) {
                TextDisplay_grab(text->display);
                text->needsGrab = 0;
        }

        if (text->needsRedraw == 1) {
                Interface_editViewText_redraw();
                text->needsRedraw = 0;
        }
}
