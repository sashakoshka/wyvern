#include <stdio.h>
#include <string.h>

#include "edit-buffer.h"
#include "options.h"

static void EditBuffer_placeLine (EditBuffer *, String *, size_t);
static void EditBuffer_realloc   (EditBuffer *, size_t);
static void EditBuffer_shiftDown (EditBuffer *, size_t, size_t);
static void EditBuffer_shiftUp   (EditBuffer *, size_t, size_t, int);

static size_t constrainChange (size_t, int, size_t);

                      /* * * * * * * * * * * * * * * *
                       * EditBuffer member functions *
                       * * * * * * * * * * * * * * * */

/* EditBuffer_new
 * Creates and initializes a new edit buffer.
 */
EditBuffer *EditBuffer_new (void) {
        EditBuffer *editBuffer = calloc(1, sizeof(EditBuffer));
        EditBuffer_reset(editBuffer);
        return editBuffer;
}

/* EditBuffer_free
 * Clears and frees an edit buffer.
 */
void EditBuffer_free (EditBuffer *editBuffer) {
        EditBuffer_reset(editBuffer);
        free(editBuffer);
}

/* EditBuffer_open
 * Loads a file into an edit buffer.
 */
Error EditBuffer_open (EditBuffer *editBuffer, const char *filePath) {
        EditBuffer_reset(editBuffer);
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
        EditBuffer_reset(editBuffer);

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

/* EditBuffer_reset
 * Resets the buffer, clearing and freeing its contents. After this function,
 * the buffer itself can be safely freed, or new content can be loaded or
 * entered in.
 */
void EditBuffer_reset (EditBuffer *editBuffer) {
        for (size_t index = 0; index < editBuffer->length; index ++) {
                String_free(editBuffer->lines[index]);
        }

        *editBuffer = (const EditBuffer) { 0 };
        EditBuffer_addNewCursor(editBuffer, 0, 0);
}

/* EditBuffer_clearExtraCursors
 * Remove all cursors except the original one.
 */
void EditBuffer_clearExtraCursors (EditBuffer *editBuffer) {
        editBuffer->amountOfCursors = 0;
}

/* EditBuffer_addNewCursor
 * Adds a new cursor at the specified row and column.
 */
void EditBuffer_addNewCursor (
        EditBuffer *editBuffer,
        size_t column,
        size_t row
) {
        // don't add a new cursor if we are at the max
        if (editBuffer->amountOfCursors >= EDITBUFFER_MAX_CURSORS - 1) {
                return;
        }
        
        editBuffer->cursors[editBuffer->amountOfCursors].parent = editBuffer;
        editBuffer->cursors[editBuffer->amountOfCursors].column = column;
        editBuffer->cursors[editBuffer->amountOfCursors].row    = row;
        editBuffer->amountOfCursors ++;
}

/* EditBuffer_Cursor_insertRune
 * Inserts a character at the current cursor position. If there are no lines in
 * the edit buffer, this function does nothing.
 */
void EditBuffer_Cursor_insertRune (EditBuffer_Cursor *cursor, Rune rune) {
        if (cursor->parent->length == 0) { return; }
        String *currentLine = EditBuffer_Cursor_getCurrentLine(cursor);

        if (rune == '\n') {
                String *newLine = String_new("");
                String_splitInto(currentLine, newLine, cursor->column);

                cursor->row ++;
                cursor->column = 0;

                EditBuffer_placeLine(cursor->parent, newLine, cursor->row);
                return;
        }

        if (rune == '\t' && Options_tabsToSpaces) {
                size_t spacesNeeded =
                        (size_t)(Options_tabSize) - (
                                cursor->column %
                                (size_t)(Options_tabSize));

                size_t spacesLeft = spacesNeeded;
                while (spacesLeft --> 0) {
                        String_insertRune(currentLine, ' ', cursor->column);
                }

                cursor->column += spacesNeeded;
                return;
        }

        String_insertRune(currentLine, rune, cursor->column);
}

/* EditBuffer_Cursor_deleteRune
 * Deletes the char at the cursor. If there are no lines in the edit buffer,
 * this function does nothing.
 */
void EditBuffer_Cursor_deleteRune (EditBuffer_Cursor *cursor) {
        if (cursor->parent->length == 0) { return; }
        String *currentLine = EditBuffer_Cursor_getCurrentLine(cursor);
        
        // if we within a line, we can just delete the rune we are on
        if (cursor->column < currentLine->length) {
                String_deleteRune(currentLine, cursor->column);
                return;
        }

        // cannot combine a line below
        if (cursor->row >= cursor->parent->length - 1) { return; }

        // lift next line out and combine it with this one
        String *nextLine = EditBuffer_getLine(cursor->parent, cursor->row + 1);
        String_addString(currentLine, nextLine);
        EditBuffer_shiftUp(cursor->parent, cursor->row + 1, 1, 0);
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

/* EditBuffer_cursorsInsertRune
 * Inserts a rune at all cursors.
 */
void EditBuffer_cursorsInsertRune (EditBuffer *editBuffer, Rune rune) {
        EditBuffer_Cursor_insertRune(&editBuffer->cursors[0], rune);
}
/* EditBuffer_cursorsInsertRune
 * Deletes a rune at all cursors.
 */
void EditBuffer_cursorsDeleteRune (EditBuffer *editBuffer) {
        EditBuffer_Cursor_deleteRune(&editBuffer->cursors[0]);
}

/* EditBuffer_cursorsMoveH
 * Moves all cursors horizontally by amount.
 */
void EditBuffer_cursorsMoveH (EditBuffer *editBuffer, int amount) {
        EditBuffer_Cursor_moveH(&editBuffer->cursors[0], amount);
}

/* EditBuffer_cursorsMoveV
 * Moves all cursors vertically by amount.
 */
void EditBuffer_cursorsMoveV (EditBuffer *editBuffer, int amount) {
        EditBuffer_Cursor_moveV(&editBuffer->cursors[0], amount);
}

// TODO
void EditBuffer_cursorsMoveWordH    (EditBuffer *editBuffer, int amount);
void EditBuffer_cursorsMoveWordV    (EditBuffer *editBuffer, int amount);

/* EditBuffer_cursorsMoveTo
 * Moves all cursors to one location.
 *
 * TODO: when multiple cursors are fully implemented, make another function to
 * check if cursors overlap eachother and delete duplicates. Run this function
 * whenever cursors change position.
 */
void EditBuffer_cursorsMoveTo ( 
        EditBuffer *editBuffer,
        size_t column,
        size_t row
) {
        EditBuffer_Cursor_moveTo(&editBuffer->cursors[0], column, row);
}

// TODO
void EditBuffer_cursorsChangeIndent (EditBuffer *, int);
void EditBuffer_cursorsInsertString (EditBuffer *, String *);

/* EditBuffer_realloc
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

        if (editBuffer->cursors[0].row >= editBuffer->length) {
                editBuffer->cursors[0].row = editBuffer->length;
        }
}

                  /* * * * * * * * * * * * * * * * * * * *
                   * EditBuffer_Cursor member functions  *
                   * * * * * * * * * * * * * * * * * * * */

/* EditBuffer_Cursor_moveH
 * Horizontally moves the cursor by amount. This function does bounds checking.
 * If the cursor runs off the line, it is taken to the previous or next line
 * (if possible),
 */
void EditBuffer_Cursor_moveH (EditBuffer_Cursor *cursor, int amount) {
        size_t lineLength;
        if (cursor->column == 0 && amount < 0) {
                if (cursor->row > 0) {
                        cursor->row --;
                        lineLength = EditBuffer_Cursor_getCurrentLine (
                                cursor
                        )->length;
                        cursor->column = lineLength;
                }
                return;
        }

        lineLength = EditBuffer_Cursor_getCurrentLine(cursor)->length;
        if (cursor->column >= lineLength && amount > 0) {
                if (cursor->row < cursor->parent->length - 1) {
                        cursor->row ++;
                        cursor->column = 0;
                }
                return;
        }

        size_t amountAbs;
        if (amount < 0) {
                amountAbs = (size_t)(0 - amount);
                cursor->column -= amountAbs;
        } else {
                amountAbs = (size_t)(amount);
                cursor->column += amountAbs;
        }
}

/* EditBuffer_Cursor_moveV
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
 *
 * actually that might be a bad idea due to multiple cursors. it would not
 * really work. possibly make a function in the unicode module that takes in a
 * column and figures out the width of a rune and use that in various places.
 */
void EditBuffer_Cursor_moveV (EditBuffer_Cursor *cursor, int amount) {
        size_t rowBefore = cursor->row;
        cursor->row = constrainChange (
                cursor->row,
                amount,
                cursor->parent->length);

        size_t lineLength = EditBuffer_Cursor_getCurrentLine(cursor)->length;
        if (cursor->row == rowBefore) {
                if (amount > 0) {
                        cursor->column = lineLength;
                } else {
                        cursor->column = 0;
                }
        }

        if (cursor->column > lineLength) {
                cursor->column = lineLength;
        }
}

// TODO
void EditBuffer_Cursor_moveWordH (EditBuffer_Cursor *cursor, int);
void EditBuffer_Cursor_moveWordV (EditBuffer_Cursor *cursor, int);

/* EditBuffer_Cursor_moveTo
 * Moves the cursor of the edit buffer to the specified row and column.
 */
void EditBuffer_Cursor_moveTo (
        EditBuffer_Cursor *cursor,
        size_t column,
        size_t row
) {
        cursor->column = column;
        cursor->row    = row;
}

// TODO
void EditBuffer_Cursor_changeIndent (EditBuffer_Cursor *cursor, int);
void EditBuffer_Cursor_insertString (EditBuffer_Cursor *cursor, String *string);

/* EditBuffer_Cursor_getCurrentLine
 * Returns a pointer to the line that the cursor is currently on.
 */
String *EditBuffer_Cursor_getCurrentLine (EditBuffer_Cursor *cursor) {
        return EditBuffer_getLine(cursor->parent, cursor->row);
}

                            /* * * * * * * * * * *
                             * Utility functions *
                             * * * * * * * * * * */

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
