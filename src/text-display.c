#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "text-display.h"
#include "options.h"

static void TextDisplay_grabRow (TextDisplay *, size_t);
static void TextDisplay_clear   (TextDisplay *);

/* TextDisplay_new
 * Creates a new text display that is modeled after editBuffer, has a width of
 * width, and a height of height.
 */
TextDisplay *TextDisplay_new (
        EditBuffer *editBuffer,
        size_t width,
        size_t height
) {
        TextDisplay *textDisplay = calloc(1, sizeof(TextDisplay));

        textDisplay->model = editBuffer;

        textDisplay->width  = width;
        textDisplay->height = height;

        textDisplay->cells = calloc(width * height, sizeof(TextDisplay_Cell));
        return textDisplay;
}

/* TextDisplay_free
 * Frees a text display and all associated data, except its model.
 */
void TextDisplay_free (TextDisplay *textDisplay) {
        free(textDisplay->cells);
        free(textDisplay);
}

/* TextDisplay_grab
 * Updates the contents of a text display, reflecting its model.
 */
void TextDisplay_grab (TextDisplay *textDisplay) {
        if (textDisplay->model == NULL) {
                TextDisplay_clear(textDisplay);
                return;
        }
                        
        for (size_t row = 0; row < textDisplay->height; row ++) {
                TextDisplay_grabRow(textDisplay, row);
        }
}

/* TextDisplay_grabRow
 * Updates a single row of a text display, reflecting its model. The row is
 * relative to the text display, not its model - the position of the text
 * display relative to the model is automatically determined based on the scroll
 * value.
 */
static void TextDisplay_grabRow (TextDisplay *textDisplay, size_t row) {
        size_t scroll = textDisplay->model->scroll;
        size_t realRow = row + scroll;

        String *line = NULL;
        if (realRow < textDisplay->model->length) {
                line = textDisplay->model->lines[realRow];
        }
        
        size_t realColumn      = 0;
        int    findNextTabStop = 0;
        for (size_t column = 0; column < textDisplay->width; column ++) {
                Rune   new             = TEXTDISPLAY_EMPTY_CELL;
                size_t coordinate      = row * textDisplay->width + column;
                int    isOwnRune       = 0;
                TextDisplay_Cell *cell = &textDisplay->cells[coordinate];

                // if we are at a tab stop, stop looking for it (if we even are) 
                if (column % (size_t)(Options_tabSize) == 0) {
                        findNextTabStop = 0;
                }

                if (findNextTabStop) {
                        // fill tabs with whitespace
                        new = ' ';
                } else if (line != NULL && realColumn < line->length) {
                        // if the cell actually maps to a rune, get it
                        new = line->buffer[realColumn];
                        isOwnRune = 1;
                }

                // line breaks need to be their own runes
                if (line != NULL && realColumn == line->length) {
                        isOwnRune = 1;
                }

                // if this is a tab, switch to looking for the next tab stop
                if (new == '\t') {
                        findNextTabStop = 1;
                        new = ' ';
                }

                // TODO: have function in EditBuffer to check if there is a
                // cursor at the specified coordinates.
                int hasCursor =
                        realRow    == textDisplay->model->cursors[0].row    &&
                        realColumn == textDisplay->model->cursors[0].column &&
                        isOwnRune;

                uint8_t damaged =
                        cell->rune != new |
                        cell->hasCursor != hasCursor;
                cell->damaged   = damaged;
                cell->rune      = new;
                cell->hasCursor = (uint8_t)hasCursor;
                
                if (realRow >= textDisplay->model->length) {
                        cell->realRow    = textDisplay->model->length - 1;
                        cell->realColumn = textDisplay->cells [
                                textDisplay->lastRow * textDisplay->width +
                                column].realColumn;
                } else {
                        cell->realRow    = realRow;
                        cell->realColumn = realColumn;

                        if (isOwnRune) {
                                textDisplay->lastRealColumn = realColumn;
                                textDisplay->lastRealRow    = realRow;
                                textDisplay->lastRow        = row;
                                realColumn ++;
                        } else {
                                cell->realColumn = textDisplay->lastRealColumn;
                                cell->realRow    = textDisplay->lastRealRow;
                        }
                }
        }
}

/* TextDisplay_setModel
 * Sets the model of a text display. When TextDisplay_grab is called, this model
 * will be grabbed from.
 */
void TextDisplay_setModel (TextDisplay *textDisplay, EditBuffer *editBuffer) {
        textDisplay->model = editBuffer;
}


/* TextDisplay_resize
 * Resizes a text display to a new width and height, and fills the buffers with
 * zero values. This will not update the contents - you need to call
 * TextDisplay_grab manuall after this function.
 */
void TextDisplay_resize (
        TextDisplay *textDisplay,
        size_t width,
        size_t height
) {
        if (width == textDisplay->width && height == textDisplay->height) {
                TextDisplay_clear(textDisplay);
                return;
        }

        textDisplay->width  = width;
        textDisplay->height = height;

        free(textDisplay->cells);
        textDisplay->cells = calloc(width * height, sizeof(TextDisplay_Cell));
}

/* TextDisplay_getRealCoords
 * Takes in the coordinates of a cell in the buffer, and outputs the row and
 * column that it points to in the model.
 */
void TextDisplay_getRealCoords (
        TextDisplay  *textDisplay,
        size_t  column,     size_t  row,
        size_t *realColumn, size_t *realRow
) {
        size_t coordinate      = row * textDisplay->width + column;
        TextDisplay_Cell *cell = &textDisplay->cells[coordinate];

        *realRow    = cell->realRow;
        *realColumn = cell->realColumn;
}

/* TextDisplay_clear
 * Clears a text display, setting all values in the buffer to zero. This does
 * not mofidy the damage buffer.
 */
static void TextDisplay_clear (TextDisplay *textDisplay) {
        memset (
                textDisplay->cells, 0,
                textDisplay->width *
                textDisplay->height * sizeof(TextDisplay_Cell));
}
