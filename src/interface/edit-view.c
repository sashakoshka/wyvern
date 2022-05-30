#include "module.h"
#include "options.h"

/* Interface_editView_recalculate
 * Recalculates the position, size, and various layout bounds of the edit view.
 */
void Interface_editView_recalculate (void) {
        Interface_EditView      *editView = &interface.editView;
        Interface_EditViewText  *text     = &editView->text;
        Interface_EditViewRuler *ruler    = &editView->ruler;
        
        editView->x      = 0;
        editView->y      = interface.tabBar.height;
        editView->height = interface.height - interface.tabBar.height;
        editView->width  = interface.width;
        
        editView->padding = (int)(interface.fonts.glyphWidth * 2);
        ruler->width      = (int)(interface.fonts.glyphWidth * 5);

        editView->innerX      = editView->x      + editView->padding;
        editView->innerY      = editView->y      + editView->padding;
        editView->innerWidth  = editView->width  - editView->padding;
        editView->innerHeight = editView->height - editView->padding;

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

/* Interface_editView_redraw
 * Completely redraws the edit view.
 */
void Interface_editView_redraw (void) {
        Interface_EditView     *editView = &interface.editView;
        Interface_EditViewText *text     = &editView->text;
        
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
        double columnMarkerX =
                text->x + interface.fonts.glyphWidth * 80 + 1;
        cairo_move_to(Window_context, columnMarkerX, editView->y);
        cairo_line_to (
                Window_context,
                columnMarkerX,
                editView->y + editView->height);
        cairo_stroke(Window_context);
}

/* Interface_editViewRuler_redraw
 * Redraws the line number ruler.
 */
void Interface_editViewRuler_redraw (void) {
        Interface_EditView      *editView = &interface.editView;
        Interface_EditViewText  *text     = &editView->text;
        Interface_EditViewRuler *ruler    = &editView->ruler;
        
        cairo_set_source_rgb(Window_context, RULER_COLOR);
        cairo_rectangle (
                Window_context,
                editView->x,
                editView->y,
                editView->innerX + ruler->width,
                editView->height);
        cairo_fill(Window_context);
        
        double y = text->y;
        for (
                size_t index = text->buffer->scroll;
                index < text->buffer->length &&
                y < text->y + text->height;
                index ++
        ) {     
                char lineNumberBuffer[8] = { 0 };
                snprintf(lineNumberBuffer, 7, "%zu", index + 1);
                
                cairo_set_source_rgb(Window_context, RULER_TEXT_COLOR);
                cairo_move_to (
                        Window_context,
                        editView->innerX, y);
                cairo_show_text(Window_context, lineNumberBuffer);

                y += interface.fonts.lineHeight;
        }
}

/* Interface_editView_drawChars
 * Updates the internal TextDisplay if grabModel is 1, and re-draws damaged
 * runes.
 */
void Interface_editView_drawChars (int grabModel) {
        Interface_EditView     *editView = &interface.editView;
        Interface_EditViewText *text     = &editView->text;
        
        if (grabModel) { TextDisplay_grab(text->display); }

        for (size_t y = 0; y < text->display->height; y ++) {
                Interface_editView_drawCharsRow(y);
        }
}

/* Interface_editView_drawCharsRow
 * Re-draws damaged characters at row y.
 */
void Interface_editView_drawCharsRow (size_t y) {
        Interface_EditView     *editView = &interface.editView;
        Interface_EditViewText *text     = &editView->text;
        int inIndent = 1;
        
        for (size_t x = 0; x < text->display->width; x ++) {
                // TODO: make a function to draw a single cell. this will also
                // be faster to just call that to blink the cursors.
                size_t coordinate = y * text->display->width + x;
                TextDisplay_Cell *cell =
                        &text->display->cells[coordinate];

                // if the cell is undamaged, we don't want to render it.
                // however, we'll make an exception for cursors because those
                // need to blink.
                if (
                        !cell->damaged &&
                        cell->cursorState != TextDisplay_CursorState_cursor
                ) { continue; }
                
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
                                continue;
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
}

/* Interface_findMouseHoverCell
 * Takes in mouse coordinates and finds out the coordinates of the cell they are
 * hovering over. If the given coordinates exceed the bounds of the text view,
 * they are constrained to it.
 */
void Interface_findMouseHoverCell (
        int mouseX,
        int mouseY,
        size_t *cellX,
        size_t *cellY
) {
        Interface_EditView     *editView = &interface.editView;
        Interface_EditViewText *text     = &editView->text;
        
        int intCellX = (int) (
                (mouseX - text->x) /
                interface.fonts.glyphWidth);
        int intCellY = (int) (
                (mouseY - editView->innerY) /
                interface.fonts.lineHeight);

        *cellX = (size_t)(intCellX);
        *cellY = (size_t)(intCellY);
        
        if (intCellX < 0) { *cellX = 0; }
        if (intCellY < 0) { *cellY = 0; }
        
        if (*cellX >= text->display->width) {
                *cellX = text->display->width - 1;
        }
        if (*cellY >= text->display->height) {
                *cellY = text->display->height - 1;
        }
}

/* Interface_updateTextSelection
 * Updates the text selection according to the mouse position. This should only
 * be called when the user is actively selecting text with the mouse.
 */
void Interface_updateTextSelection (void) {
        Interface_EditView     *editView = &interface.editView;
        Interface_EditViewText *text     = &editView->text;
        
        size_t cellX = 0;
        size_t cellY = 0;
        Interface_findMouseHoverCell (
                interface.mouseState.x, interface.mouseState.y,
                &cellX, &cellY);
        
        size_t realX;
        size_t realY;
        TextDisplay_getRealCoords (
                text->display,
                cellX, cellY,
                &realX, &realY);

        EditBuffer_Cursor_moveTo (
                text->buffer->cursors,
                interface.mouseState.dragOriginRealX,
                interface.mouseState.dragOriginRealY);
        EditBuffer_Cursor_selectTo (
                text->buffer->cursors,
                realX, realY);
}

/* Interface_editView_refresh
 * Refreshes the edit view.
 */
void Interface_editView_refresh (void) {
        if (interface.editView.needsRecalculate == 1) {
                Interface_editView_recalculate();
                interface.editView.needsRecalculate = 0;
        }

        if (interface.editView.needsRedraw == 1) {
                Interface_editView_redraw();
                interface.editView.needsRedraw = 0;
        }

        // Interface_editViewRuler_refresh();
        // Interface_editViewText_refresh();
}

/* Interface_editView_invalidateLayout
 * Invalidates the layout of the edit view.
 */
void Interface_editView_invalidateLayout (void) {
        Interface_EditView *editView = &interface.editView;
        
        editView->needsRedraw = 1;
        editView->needsRecalculate = 1;
        Interface_editViewRuler_invalidateLayout();
        Interface_editViewText_invalidateLayout();
}

/* Interface_editView_invalidateDrawing
 * Invalidates the drawing of the edit view.
 */
void Interface_editView_invalidateDrawing (void) {
        Interface_EditView *editView = &interface.editView;
        
        editView->needsRedraw = 1;
        Interface_editViewRuler_invalidateDrawing();
        Interface_editViewText_invalidateDrawing();
}

/* Interface_editViewText_invalidateLayout
 * Invalidates the layout of the text.
 */
void Interface_editViewText_invalidateLayout (void) {
        Interface_EditViewText *text = &interface.editView.text;
        text->needsRedraw = 1;
        text->needsRecalculate = 1;
}

/* Interface_editViewText_invalidateDrawing
 * Invalidates the drawing of the text.
 */
void Interface_editViewText_invalidateDrawing (void) {
        Interface_EditViewText *text = &interface.editView.text;
        text->needsRedraw = 1;
}

/* Interface_editViewRuler_invalidateLayout
 * Invalidates the layout of the ruler.
 */
void Interface_editViewRuler_invalidateLayout (void) {
        Interface_EditViewRuler *ruler = &interface.editView.ruler;
        ruler->needsRedraw = 1;
        ruler->needsRecalculate = 1;
}

/* Interface_editViewRuler_invalidateDrawing
 * Invalidates the drawing of the ruler.
 */
void Interface_editViewRuler_invalidateDrawing (void) {
        Interface_EditViewRuler *ruler = &interface.editView.ruler;
        ruler->needsRedraw = 1;
}
