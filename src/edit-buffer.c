#include <stdio.h>
#include <string.h>

#include "edit-buffer.h"
#include "options.h"

static void EditBuffer_placeLine (EditBuffer *, String *, size_t);
static void EditBuffer_realloc   (EditBuffer *, size_t);
static void EditBuffer_shiftDown (EditBuffer *, size_t, size_t);
static void EditBuffer_shiftUp   (EditBuffer *, size_t, size_t, int);

static size_t constrainChange (size_t, int, size_t);

/* EditBuffer_new
 * Creates and initializes a new edit buffer.
 */
EditBuffer *EditBuffer_new (void) {
        EditBuffer *editBuffer = calloc(1, sizeof(EditBuffer));
        return editBuffer;
}

/* EditBuffer_free
 * Clears and frees an edit buffer.
 */
void EditBuffer_free (EditBuffer *editBuffer) {
        EditBuffer_clear(editBuffer);
        free(editBuffer);
}

/* EditBuffer_open
 * Loads a file into an edit buffer.
 */
Error EditBuffer_open (EditBuffer *editBuffer, const char *filePath) {
        EditBuffer_clear(editBuffer);
        strncpy(editBuffer->filePath, filePath, PATH_MAX);

        FILE *file = fopen(filePath, "r");
        if (file == NULL) { return Error_cantOpenFile; }

        String *line = String_new("");
        EditBuffer_placeLine (
                editBuffer, line,
                editBuffer->length);
        int  reachedEnd = 0;
        while (!reachedEnd) {
                Rune rune = Unicode_utf8FileGetRune(file, &reachedEnd);
                if (rune == 0) { continue; }
                
                if (rune == '\n') {
                        line = String_new("");
                        EditBuffer_placeLine (
                                editBuffer, line,
                                editBuffer->length);
                } else {
                        String_addRune(line, rune);
                }
        }

        fclose(file);
        return Error_cantOpenFile;
}

/* EditBuffer_copy
 * Copies in a char buffer into an edit buffer.
 */
void EditBuffer_copy (EditBuffer *editBuffer, const char *buffer) {
        EditBuffer_clear(editBuffer);

        String *line = String_new("");
        char ch;
        for (size_t index = 0; (ch = buffer[index]); index ++) {
                if (ch == '\n') {
                        EditBuffer_placeLine (
                                editBuffer, line,
                                editBuffer->length);
                        line = String_new("");
                } else {
                        // TODO: fix this!
                        String_addRune(line, (Rune)ch);
                }
        }
}

/* EditBuffer_clear
 * Resets the buffer, clearing and freeing its contents. After this function,
 * the buffer itself can be safely freed, or new content can be loaded or
 * entered in.
 */
void EditBuffer_clear (EditBuffer *editBuffer) {
        for (size_t index = 0; index < editBuffer->length; index ++) {
                String_free(editBuffer->lines[index]);
        }

        *editBuffer = (const EditBuffer) { 0 };
}

/* EditBuffer_insertRune
 * Inserts a character at the current cursor position. If there are no lines in
 * the edit buffer, this function does nothing.
 */
void EditBuffer_insertRune (EditBuffer *editBuffer, Rune rune) {
        if (editBuffer->length == 0) { return; }

        if (rune == '\n') {
                String *currentLine = EditBuffer_getCurrentLine(editBuffer);
                String *newLine     = String_new("");
                String_splitInto(currentLine, newLine, editBuffer->column);

                editBuffer->row ++;
                editBuffer->column = 0;

                EditBuffer_placeLine(editBuffer, newLine, editBuffer->row);
                return;
        }

        if (rune == '\t' && Options_tabsToSpaces) {
                size_t spacesNeeded =
                        (size_t)(Options_tabSize) - (
                                editBuffer->column %
                                (size_t)(Options_tabSize));
                
                while (spacesNeeded --> 0) {
                        String_insertRune (
                                editBuffer->lines[editBuffer->row],
                                ' ', editBuffer->column);
                }
                return;
        }

        String_insertRune (
                editBuffer->lines[editBuffer->row],
                rune, editBuffer->column);
}

/* EditBuffer_deleteRune
 * Deletes the char at the cursor. If there are no lines in the edit buffer,
 * this function does nothing.
 */
void EditBuffer_deleteRune (EditBuffer *editBuffer) {
        if (editBuffer->length == 0) { return; }

        String *currentLine = EditBuffer_getCurrentLine(editBuffer);
        // if we within a line, we can just delete the rune we are on
        if (editBuffer->column < currentLine->length) {
                String_deleteRune(currentLine, editBuffer->column);
                return;
        }

        // cannot combine a line below
        if (editBuffer->row >= editBuffer->length - 1) { return; }

        // lift next line out and combine it with this one
        String *nextLine = EditBuffer_getLine(editBuffer, editBuffer->row + 1);
        String_addString(currentLine, nextLine);
        EditBuffer_shiftUp(editBuffer, editBuffer->row + 1, 1, 0);
}

/* EditBuffer_scroll
 * Scrolls the edit buffer by amount. This function does bounds checking.
 */
void EditBuffer_scroll (EditBuffer *editBuffer, int amount) {
        editBuffer->scroll = constrainChange (
                editBuffer->scroll,
                amount,
                editBuffer->length);
}

/* EditBuffer_getLine
 * Returns the line at row.
 */
String *EditBuffer_getLine (EditBuffer *editBuffer, size_t row) {
        return editBuffer->lines[row];
}

/* EditBuffer_getCurrentLine
 * Returns the current line that the cursor is on.
 */
String *EditBuffer_getCurrentLine (EditBuffer *editBuffer) {
        return EditBuffer_getLine(editBuffer, editBuffer->row);
}

/* TODO
 * Make new struct called EditBuffer_Cursor, and make all functions that depend
 * on cursor position (such as text insertion, deletion, and movement) members
 * of that struct. Also make functions to do it to all of them at once.
 */

/* EditBuffer_cursorMoveH
 * Horizontally moves the cursor by amount. This function does bounds checking.
 * If the cursor runs off the line, it is taken to the previous or next line
 * (if possible),
 */
void EditBuffer_cursorMoveH (EditBuffer *editBuffer, int amount) {
        size_t lineLength;
        if (editBuffer->column == 0 && amount < 0) {
                if (editBuffer->row > 0) {
                        editBuffer->row --;
                        lineLength =
                                EditBuffer_getCurrentLine(editBuffer)->length;
                        editBuffer->column = lineLength;
                }
                return;
        }

        lineLength = EditBuffer_getCurrentLine(editBuffer)->length;
        if (editBuffer->column >= lineLength && amount > 0) {
                if (editBuffer->row < editBuffer->length - 1) {
                        editBuffer->row ++;
                        editBuffer->column = 0;
                }
                return;
        }

        size_t amountAbs;
        if (amount < 0) {
                amountAbs = (size_t)(0 - amount);
                editBuffer->column -= amountAbs;
        } else {
                amountAbs = (size_t)(amount);
                editBuffer->column += amountAbs;
        }
}

/* EditBuffer_cursorMoveV
 * Vertically moves the cursor by amount. This function does bounds checking.
 *
 * TODO: make this somewhat dependant on TextDisplay without introducing a
 * dependancy cycle, perhapas just have TextDisplay handle it. It is nescessary
 * to do this because vertical cursor movement needs to cross wrapped lines and
 * account for runes larger than one cell so the movement makes sense to the
 * user. The new function should follow these steps:
 *
 * 1. check if the buffer needs to be scrolled (if the cursor is at the edge)
 * 2. if so, scroll it first
 * 3. go up/down a row
 * 4. set new cursor position to the real position of that character
 */
void EditBuffer_cursorMoveV (EditBuffer *editBuffer, int amount) {
        size_t rowBefore = editBuffer->row;
        editBuffer->row = constrainChange (
                editBuffer->row,
                amount,
                editBuffer->length);

        size_t lineLength = EditBuffer_getCurrentLine(editBuffer)->length;
        if (editBuffer->row == rowBefore) {
                if (amount > 0) {
                        editBuffer->column = lineLength;
                } else {
                        editBuffer->column = 0;
                }
        }

        if (editBuffer->column > lineLength) {
                editBuffer->column = lineLength;
        }
}

void EditBuffer_cursorMoveWordH (EditBuffer *editBuffer, int);
void EditBuffer_cursorMoveWordV (EditBuffer *editBuffer, int);

/* EditBuffer_cursorMoveTo
 * Moves the cursor of the edit buffer to the specified row and column.
 */
void EditBuffer_cursorMoveTo (
        EditBuffer *editBuffer,
        size_t column,
        size_t row
) {
        editBuffer->column = column;
        editBuffer->row    = row;
}

void EditBuffer_changeIndent (EditBuffer *editBuffer, int);
void EditBuffer_insertBuffer (EditBuffer *editBuffer, const char *);

/* EditBuffer_placeLine
 * Inserts a line at the specified index, moving all lines after it downwards.
 */
static void EditBuffer_placeLine (
        EditBuffer *editBuffer,
        String     *line,
        size_t     index
) {
        EditBuffer_shiftDown(editBuffer, index, 1);
        editBuffer->lines[index] = line;
}

/* EditBuffer_shiftDown
 * Shifts the contents of the edit buffer down after index, leaving a gap of
 * NULL pointers. This reallocates the buffer to accommodate the gap.
 */
static void EditBuffer_shiftDown (
        EditBuffer *editBuffer,
        size_t     index,
        size_t     amount
) {
        size_t end       = editBuffer->length + amount;
        size_t beginning = index + amount - 1;
        EditBuffer_realloc(editBuffer, end);

        for (size_t current = end - 1; current > beginning; current --) {
                size_t target = current - amount;
                editBuffer->lines[current] = editBuffer->lines[target];
                editBuffer->lines[target] = NULL;
        }
}

/* EditBuffer_shiftUp
 * Shifts the contents of the edit buffer up after index. If keep is 0, the
 * overwritten lines will be freed. Unless something else needs to be done with
 * them, this should always be set to 0!
 */
static void EditBuffer_shiftUp (
        EditBuffer *editBuffer,
        size_t     index,
        size_t     amount,
        int        keep
) {
        size_t end      = editBuffer->length - amount;
        size_t toDelete = amount;
        for (; index < end; index ++) {
                if (!keep && toDelete > 0) {
                        String_free(editBuffer->lines[index]);
                        toDelete --;
                }
                editBuffer->lines[index] = editBuffer->lines[index + amount];
        }
        
        EditBuffer_realloc(editBuffer, end);
}

/* String_realloc
 * Resizes the internal buffer of the edit buffer to accomodate a file of
 * newLength lines.
 */
static void EditBuffer_realloc (EditBuffer *editBuffer, size_t newLength) {
        if (newLength == editBuffer->length) { return; }

        if (newLength < editBuffer->size) {
                // if the buffer is shrinking, just set the size to the new
                // size.
                editBuffer->size = newLength;
        } else {
                // try multiplying the current size by 2
                editBuffer->size *= 2;
                // if that isn't enough, just exactly match the new size.
                if (newLength > editBuffer->size) {
                        editBuffer->size = newLength;
                }
        }
        
        editBuffer->lines = realloc (
                editBuffer->lines,
                editBuffer->size * sizeof(String *));
        editBuffer->length = newLength;

        // make sure cursor parameters are within bounds
        if (editBuffer->scroll >= editBuffer->length) {
                editBuffer->scroll = editBuffer->length;
        }

        if (editBuffer->row >= editBuffer->length) {
                editBuffer->row = editBuffer->length;
        }
}

/* constrainChange
 * Constrains a change in initial of amount, to a lower bound of 0 and an upper
 * bound of bound (bound is not inclusive).
 */
static size_t constrainChange (size_t initial, int amount, size_t bound) {
        size_t amountAbs;
        
        if (amount < 0) {
                amountAbs = (size_t)(0 - amount);
                if (initial >= amountAbs) {
                        initial -= amountAbs;
                } else {
                        initial = 0;
                }
        } else {
                amountAbs = (size_t)amount;
                initial += amountAbs;
        }
        
        if (initial >= bound) {
                initial = bound - 1;
        }

        return initial;
}
