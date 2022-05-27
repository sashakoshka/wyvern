#include "module.h"

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

void Interface_editView_drawChars (int grabModel) {
        if (grabModel) { TextDisplay_grab(textDisplay); }

        for (size_t y = 0; y < textDisplay->height; y ++) {
                Interface_editView_drawCharsRow(y);
        }
}

void Interface_editView_drawCharsRow (size_t y) {
        Interface_EditView *editView = &interface.editView;
        int inIndent = 1;
        
        for (size_t x = 0; x < textDisplay->width; x ++) {
                size_t coordinate = y * textDisplay->width + x;
                TextDisplay_Cell *cell = &textDisplay->cells[coordinate];

                // if the cell is undamaged, we don't want to render it.
                // however, we'll make an exception for cursors because those
                // need to blink.
                if (
                        !cell->damaged &&
                        cell->cursorState != TextDisplay_CursorState_cursor
                ) { continue; }
                
                textDisplay->cells[coordinate].damaged = 0;
                
                double realX = editView->textX;
                double realY = editView->innerY;
                realX += (double)(x) * glyphWidth;
                realY += (double)(y) * lineHeight;

                // background to clear what was previously there
                cairo_set_source_rgb(Window_context, BACKGROUND_COLOR);
                cairo_rectangle (
                        Window_context,
                        realX, realY,
                        glyphWidth, lineHeight);
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
                                realY + glyphHeight);
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
                                realY + lineHeight);
                        cairo_stroke(Window_context);
                }

                // selection highlight
                if (cell->cursorState == TextDisplay_CursorState_selection) {
                        cairo_set_source_rgb(Window_context, SELECTION_COLOR);
                        cairo_rectangle (
                                Window_context,
                                realX, realY,
                                glyphWidth, glyphHeight);
                        cairo_fill(Window_context);
                }

                // don't attempt to render whitespace
                if (cell->rune != TEXTDISPLAY_EMPTY_CELL && !isSpace) {
                        unsigned int index = FT_Get_Char_Index (
                                freetypeFaceNormal,
                                cell->rune);

                        // if we couldn't find the character, display a red
                        // error symbol
                        if (index == 0) {
                                cairo_set_source_rgb (
                                        Window_context,
                                        BAD_CHAR_COLOR);
                                double scale   = glyphWidth / 3;
                                double centerX = realX + glyphWidth / 2;
                                double centerY = realY + glyphHeight / 2;
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
                                .y     = realY + glyphHeight * 0.8
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
                                realY + glyphHeight);
                        cairo_stroke(Window_context);
                }
        }
}

void Interface_findMouseHoverCell (
        int mouseX,
        int mouseY,
        size_t *cellX,
        size_t *cellY
) {
        int intCellX = (int) (
                (mouseX - interface.editView.textX) /
                glyphWidth);
        int intCellY = (int) (
                (mouseY - interface.editView.innerY) /
                lineHeight);

        *cellX = (size_t)(intCellX);
        *cellY = (size_t)(intCellY);
        
        if (intCellX < 0) { *cellX = 0; }
        if (intCellY < 0) { *cellY = 0; }
        
        if (*cellX >= textDisplay->width) {
                *cellX = textDisplay->width - 1;
        }
        if (*cellY >= textDisplay->height) {
                *cellY = textDisplay->height - 1;
        }
}

void Interface_updateTextSelection (void) {
        size_t cellX = 0;
        size_t cellY = 0;
        Interface_findMouseHoverCell (
                mouseState.x, mouseState.y,
                &cellX, &cellY);
        
        size_t realX;
        size_t realY;
        TextDisplay_getRealCoords (
                textDisplay,
                cellX, cellY,
                &realX, &realY);

        EditBuffer_Cursor_moveTo (
                editBuffer->cursors,
                mouseState.dragOriginRealX, mouseState.dragOriginRealY);
        EditBuffer_Cursor_selectTo (
                editBuffer->cursors,
                realX, realY);
}
