#include "module.h"
#include "options.h"

/* Interface_editView_recalculate
 * Recalculates the position, size, and various layout bounds of the edit view.
 */
void Interface_editView_recalculate (void) {
        Interface_EditView *editView = &interface.editView;
        
        editView->x      = 0;
        editView->y      = interface.tabBar.height;
        editView->height = interface.height - interface.tabBar.height;
        editView->width  = interface.width;
        
        editView->padding    = (int)(interface.fonts.glyphWidth * 2);
        editView->rulerWidth = (int)(interface.fonts.glyphWidth * 5);

        editView->innerX      = editView->x      + editView->padding;
        editView->innerY      = editView->y      + editView->padding;
        editView->innerWidth  = editView->width  - editView->padding;
        editView->innerHeight = editView->height - editView->padding;

        double textLeftOffset = editView->rulerWidth + editView->padding;
        editView->textX = editView->innerX + textLeftOffset;
        editView->textY = editView->innerY + interface.fonts.glyphHeight * 0.8;
        editView->textWidth  = editView->width  - textLeftOffset;
        editView->textHeight = editView->height - editView->padding +
                interface.fonts.lineHeight;

        double textDisplayWidth =
                editView->textWidth  / interface.fonts.glyphWidth;
        double textDisplayHeight =
                editView->textHeight / interface.fonts.lineHeight;
        if (textDisplayWidth  < 0) { textDisplayWidth  = 0; }
        if (textDisplayHeight < 0) { textDisplayHeight = 0; }
        TextDisplay_resize (
                editView->textDisplay,
                (size_t)(textDisplayWidth),
                (size_t)(textDisplayHeight));
}

/* Interface_editView_redraw
 * Completely redraws the edit view.
 */
void Interface_editView_redraw (void) {
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
        double columnMarkerX =
                editView->textX + interface.fonts.glyphWidth * 80 + 1;
        cairo_move_to(Window_context, columnMarkerX, editView->y);
        cairo_line_to (
                Window_context,
                columnMarkerX,
                editView->y + editView->height);
        cairo_stroke(Window_context);

        Interface_editView_drawRuler();
        Interface_editView_drawChars(1);
}

/* Interface_editView_drawRuler
 * Redraws the line number ruler.
 */
void Interface_editView_drawRuler (void) {
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
                size_t index = editView->editBuffer->scroll;
                index < editView->editBuffer->length &&
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

                y += interface.fonts.lineHeight;
        }
}

/* Interface_editView_drawChars
 * Updates the internal TextDisplay if grabModel is 1, and re-draws damaged
 * runes.
 */
void Interface_editView_drawChars (int grabModel) {
        Interface_EditView *editView = &interface.editView;
        if (grabModel) { TextDisplay_grab(editView->textDisplay); }

        for (size_t y = 0; y < editView->textDisplay->height; y ++) {
                Interface_editView_drawCharsRow(y);
        }
}

/* Interface_editView_drawCharsRow
 * Re-draws damaged characters at row y.
 */
void Interface_editView_drawCharsRow (size_t y) {
        Interface_EditView *editView = &interface.editView;
        int inIndent = 1;
        
        for (size_t x = 0; x < editView->textDisplay->width; x ++) {
                // TODO: make a function to draw a single cell. this will also
                // be faster to just call that to blink the cursors.
                size_t coordinate = y * editView->textDisplay->width + x;
                TextDisplay_Cell *cell =
                        &editView->textDisplay->cells[coordinate];

                // if the cell is undamaged, we don't want to render it.
                // however, we'll make an exception for cursors because those
                // need to blink.
                if (
                        !cell->damaged &&
                        cell->cursorState != TextDisplay_CursorState_cursor
                ) { continue; }
                
                editView->textDisplay->cells[coordinate].damaged = 0;
                
                double realX = editView->textX;
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
                        editView->cursorBlink
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
        Interface_EditView *editView = &interface.editView;
        
        int intCellX = (int) (
                (mouseX - interface.editView.textX) /
                interface.fonts.glyphWidth);
        int intCellY = (int) (
                (mouseY - interface.editView.innerY) /
                interface.fonts.lineHeight);

        *cellX = (size_t)(intCellX);
        *cellY = (size_t)(intCellY);
        
        if (intCellX < 0) { *cellX = 0; }
        if (intCellY < 0) { *cellY = 0; }
        
        if (*cellX >= editView->textDisplay->width) {
                *cellX = editView->textDisplay->width - 1;
        }
        if (*cellY >= editView->textDisplay->height) {
                *cellY = editView->textDisplay->height - 1;
        }
}

/* Interface_updateTextSelection
 * Updates the text selection according to the mouse position. This should only
 * be called when the user is actively selecting text with the mouse.
 */
void Interface_updateTextSelection (void) {
        Interface_EditView *editView = &interface.editView;
        
        size_t cellX = 0;
        size_t cellY = 0;
        Interface_findMouseHoverCell (
                interface.mouseState.x, interface.mouseState.y,
                &cellX, &cellY);
        
        size_t realX;
        size_t realY;
        TextDisplay_getRealCoords (
                editView->textDisplay,
                cellX, cellY,
                &realX, &realY);

        EditBuffer_Cursor_moveTo (
                editView->editBuffer->cursors,
                interface.mouseState.dragOriginRealX,
                interface.mouseState.dragOriginRealY);
        EditBuffer_Cursor_selectTo (
                editView->editBuffer->cursors,
                realX, realY);
}
