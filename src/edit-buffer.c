#include <stdio.h>
#include <string.h>

#include "edit-buffer.h"
#include "options.h"

#define START_ALL_CURSORS                         \
        for (                                      \
                size_t index = 0;                   \
                index < editBuffer->amountOfCursors; \
                index ++                              \
        ) {                                            \
                EditBuffer_Cursor *cursor = editBuffer->cursors + index;
#define START_ALL_CURSORS_BATCH_OPERATION START_ALL_CURSORS \
        editBuffer->dontMerge = 1;

#define END_ALL_CURSORS }
#define END_ALL_CURSORS_BATCH_OPERATION END_ALL_CURSORS \
        editBuffer->dontMerge = 0;

static void EditBuffer_placeLine      (EditBuffer *, String *, size_t);
static void EditBuffer_realloc        (EditBuffer *, size_t);
static void EditBuffer_shiftDown      (EditBuffer *, size_t, size_t);
static void EditBuffer_shiftUp        (EditBuffer *, size_t, size_t, int);
static void EditBuffer_shiftCursorsInLineAfter (
        EditBuffer *,
        size_t, size_t,
        int);
static void EditBuffer_mergeCursors   (EditBuffer *);
static void EditBuffer_cursorsWrangle (EditBuffer *);
static void EditBuffer_Cursor_wrangle (EditBuffer_Cursor *);

static void EditBuffer_Cursor_predictMovement (
        EditBuffer_Cursor *,
        size_t *, size_t *,
        int, int);

static size_t constrainChange (size_t, int, size_t);
static size_t addToSizeT      (size_t, int);

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
        int reachedEnd = 0;
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
        editBuffer->amountOfCursors = 1;
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

        // don't add if there is already a cursor in that spot
        START_ALL_CURSORS
                if (cursor->column == column && cursor->row == row) {
                        return;
                }
        END_ALL_CURSORS

        EditBuffer_Cursor *newCursor =
                &editBuffer->cursors[editBuffer->amountOfCursors];
        
        newCursor->parent = editBuffer;
        newCursor->column = column;
        newCursor->row    = row;

        EditBuffer_Cursor_selectNone(newCursor);
        editBuffer->amountOfCursors ++;
}

/* EditBuffer_hasCursorAt
 * Returns 1 if there is a cursor at the specified coordinates, otherwise
 * returns zero.
 */
int EditBuffer_hasCursorAt (EditBuffer *editBuffer, size_t column, size_t row) {
        for (
                size_t cursor = 0;
                cursor < editBuffer->amountOfCursors;
                cursor ++
        ) {
                if (
                        editBuffer->cursors[cursor].column == column &&
                        editBuffer->cursors[cursor].row    == row
                ) {
                        return 1;
                }
        }

        return 0;
}

/* EditBuffer_hasSelectionAt
 * Returns 1 if there is a selection at the specified coordinates, otherwise
 * returns zero.
 */
int EditBuffer_hasSelectionAt (
        EditBuffer *editBuffer,
        size_t column,
        size_t row
) {
        START_ALL_CURSORS
                if (!cursor->hasSelection) { continue; }

                // sort selection start and end
                size_t startRow;
                size_t startColumn;
                size_t endRow;
                size_t endColumn;

                EditBuffer_Cursor_getSelectionBounds (
                        cursor,
                        &startRow, &startColumn,
                        &endRow,   &endColumn);

                // go on to the next cursor if the input is out of bounds of
                // this one
                if (row < startRow  || endRow < row)          { continue; }
                if (row == startRow && column < startColumn ) { continue; }
                if (row == endRow   && column > endColumn )   { continue; }
                               
                return 1;
        END_ALL_CURSORS
        return 0;
}

/* EditBuffer_insertRuneAt
 * This function inserts a rune at a specific column and row. This should be
 * used for cursor functionality, and for advanced programmatic text
 * modification.
 */
void EditBuffer_insertRuneAt (
        EditBuffer *editBuffer,
        size_t column, size_t row,
        Rune rune
) {
        String *currentLine = EditBuffer_getLine(editBuffer, row);

        if (rune == '\n') {
                // fancy things relating to line breaks
                String *newLine = String_new("");
                String_splitInto(currentLine, newLine, column);
                EditBuffer_placeLine(editBuffer, newLine, row + 1);
                
                START_ALL_CURSORS
                        // shift down cursors after the new line break
                        if (cursor->row == row) {
                                if (cursor->column > column) {
                                        // this cursor was previously on the
                                        // part of the line that got split and
                                        // made into its own line. it needs to
                                        // have its position set to the proper
                                        // place on that new line.
                                        cursor->row ++;
                                        cursor->column -= currentLine->length;
                                } else if (cursor->column == column) {
                                        // this is the cursor that caused the
                                        // insertion, so it should wrap around
                                        // to the beginning of the next line
                                        cursor->row ++;
                                        cursor->column = 0;
                                }
                        } else if (cursor->row > row) {
                                cursor->row ++;
                        }
                END_ALL_CURSORS
                return;
        }

        if (rune == '\t' && Options_tabsToSpaces) {
                // indent with multiple spaces if the option for this is set
                size_t spacesNeeded =
                        (size_t)(Options_tabSize) - (
                                column % (size_t)(Options_tabSize));

                size_t spacesLeft = spacesNeeded;
                while (spacesLeft --> 0) {
                        String_insertRune(currentLine, ' ', column);
                }

                EditBuffer_shiftCursorsInLineAfter (
                        editBuffer,
                        column, row,
                        (int)(spacesNeeded));
                return;
        }

        // This is just a normal rune insertion
        String_insertRune(currentLine, rune, column);
        EditBuffer_shiftCursorsInLineAfter(editBuffer, column, row, 1);
}

/* EditBuffer_deleteRuneAt
 * This function deletes the rune at a specific column and row. This should be
 * used for cursor functionality, and for advanced programmatic text
 * modification.
 */
void EditBuffer_deleteRuneAt (
        EditBuffer *editBuffer,
        size_t column, size_t row
) {
        String *currentLine = EditBuffer_getLine(editBuffer, row);
        
        // if we are within a line, we can just delete the rune we are on
        if (column < currentLine->length) {
                String_deleteRune(currentLine, column);
                EditBuffer_shiftCursorsInLineAfter (
                        editBuffer,
                        column + 1, row,
                        -1);
                return;
        }

        // cannot combine a line below
        if (row >= editBuffer->length - 1) { return; }

        // lift next line out and combine it with this one
        size_t previousLength = currentLine->length;
        String *nextLine = EditBuffer_getLine(editBuffer, row + 1);
        String_addString(currentLine, nextLine);
        EditBuffer_shiftUp(editBuffer, row + 1, 1, 0);

        START_ALL_CURSORS
                // shift up cursors under the current line
                if (cursor->row > row) {
                        cursor->row --;
                        if (cursor->row == row) {
                                // if the cursor was on a line that got merged
                                // in, we need to shift the position over to the
                                // right after it gets shifted up.
                                cursor->column += previousLength;
                        }
                }
        END_ALL_CURSORS
}

/* EditBuffer_shiftCursorsInLineAfter
 * Shifts all cursors in row that are positioned at or equal to column by
 * amount. This function should ONLY be used in rune insertion and deletion, and
 * not as a replacement for EditBuffer_Cursor_moveH.
 */
static void EditBuffer_shiftCursorsInLineAfter (
        EditBuffer *editBuffer,
        size_t column, size_t row,
        int amount
) {
        START_ALL_CURSORS
                if (cursor->row == row && cursor->column >= column) {
                        cursor->column = addToSizeT(cursor->column, amount);
                }
        END_ALL_CURSORS
}

/* EditBuffer_mergeCursors
 * Removes redundant, overlapping cursors. This should be called whenever a
 * cursor moves.
 */
static void EditBuffer_mergeCursors (EditBuffer *editBuffer) {
        // if we are currently looping over all cursors, don't merge yet
        if (editBuffer->dontMerge) { return; }

        START_ALL_CURSORS
                for (
                        size_t secondCursorIndex = index + 1;
                        secondCursorIndex < editBuffer->amountOfCursors;
                        secondCursorIndex ++
                ) {
                        EditBuffer_Cursor *secondCursor =
                                editBuffer->cursors + secondCursorIndex;
                        
                        if (secondCursor->parent == NULL) { continue; }
                        if (secondCursor == cursor)       { continue; }
                        
                        if (
                                secondCursor->row    == cursor->row &&
                                secondCursor->column == cursor->column
                        ) {
                                secondCursor->parent = NULL;
                        }
                }
        END_ALL_CURSORS

        START_ALL_CURSORS
                if (cursor->parent != NULL) { continue; }
                EditBuffer_Cursor *previousCursor = cursor;

                for (
                        size_t secondCursorIndex = 1;
                        secondCursorIndex < editBuffer->amountOfCursors;
                        secondCursorIndex ++
                ) {
                        EditBuffer_Cursor *secondCursor =
                                editBuffer->cursors + secondCursorIndex;

                        *previousCursor = *secondCursor;
                        previousCursor = secondCursor;
                }
                editBuffer->amountOfCursors --;
                index --;
        END_ALL_CURSORS
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
 * Returns the line at row. If it does not exist, this function returns NULL.
 */
String *EditBuffer_getLine (EditBuffer *editBuffer, size_t row) {
        if (row >= editBuffer->length) { return NULL; }
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
 * Shifts the contents of the edit buffer down after location, leaving a gap of
 * NULL pointers. This reallocates the buffer to accommodate the gap.
 */
static void EditBuffer_shiftDown (
        EditBuffer *editBuffer,
        size_t     location,
        size_t     amount
) {
        size_t end       = editBuffer->length + amount;
        size_t beginning = location + amount - 1;
        EditBuffer_realloc(editBuffer, end);

        for (size_t current = end - 1; current > beginning; current --) {
                size_t target = current - amount;
                editBuffer->lines[current] = editBuffer->lines[target];
                editBuffer->lines[target] = NULL;
        }
}

/* EditBuffer_shiftUp
 * Shifts the contents of the edit buffer up after location. If keep is 0, the
 * overwritten lines will be freed. Unless something else needs to be done with
 * them, this should always be set to 0!
 */
static void EditBuffer_shiftUp (
        EditBuffer *editBuffer,
        size_t     location,
        size_t     amount,
        int        keep
) {
        size_t end      = editBuffer->length - amount;
        size_t toDelete = amount;
        for (size_t index = location; index < end; index ++) {
                if (!keep && toDelete > 0) {
                        String_free(editBuffer->lines[index]);
                        toDelete --;
                }
                editBuffer->lines[index] =
                        editBuffer->lines[index + amount];
        }
        
        EditBuffer_realloc(editBuffer, end);

        // since we deleted lines, there might be cursors that are now out of
        // bounds, and we need to bring them back in.
        EditBuffer_cursorsWrangle(editBuffer);
}

/* EditBuffer_cursorsInsertRune
 * Inserts a rune at all cursors.
 */
void EditBuffer_cursorsInsertRune (EditBuffer *editBuffer, Rune rune) {
        START_ALL_CURSORS_BATCH_OPERATION
                EditBuffer_Cursor_insertRune(cursor, rune);
        END_ALL_CURSORS_BATCH_OPERATION
}

/* EditBuffer_cursorsInsertRune
 * Deletes a rune at all cursors.
 */
void EditBuffer_cursorsDeleteRune (EditBuffer *editBuffer) {
        START_ALL_CURSORS_BATCH_OPERATION
                EditBuffer_Cursor_deleteRune(cursor);
        END_ALL_CURSORS_BATCH_OPERATION
        EditBuffer_mergeCursors(editBuffer);
}

/* EditBuffer_cursorsBackspaceRune
 * Moves all cursors back once, and then deletes the runes at the cursors.
 * Cursors that cannot be moved back do not delete a rune.
 */
void EditBuffer_cursorsBackspaceRune (EditBuffer *editBuffer) {
        START_ALL_CURSORS_BATCH_OPERATION
                EditBuffer_Cursor_backspaceRune(cursor);
        END_ALL_CURSORS_BATCH_OPERATION
        EditBuffer_mergeCursors(editBuffer);
}

/* EditBuffer_cursorsMoveH
 * Moves all cursors horizontally by amount.
 */
void EditBuffer_cursorsMoveH (EditBuffer *editBuffer, int amount) {
        START_ALL_CURSORS_BATCH_OPERATION
                EditBuffer_Cursor_moveH(cursor, amount);
        END_ALL_CURSORS_BATCH_OPERATION
        EditBuffer_mergeCursors(editBuffer);
}

/* EditBuffer_cursorsMoveV
 * Moves all cursors vertically by amount.
 */
void EditBuffer_cursorsMoveV (EditBuffer *editBuffer, int amount) {
        START_ALL_CURSORS_BATCH_OPERATION
                EditBuffer_Cursor_moveV(cursor, amount);
        END_ALL_CURSORS_BATCH_OPERATION
        EditBuffer_mergeCursors(editBuffer);
}

// TODO
void EditBuffer_cursorsMoveWordH (EditBuffer *editBuffer, int amount);
void EditBuffer_cursorsMoveMoreV (EditBuffer *editBuffer, int amount);

/* EditBuffer_cursorsSelectH
 * Extends the selection of all cursors horizontally by amount
 */
void EditBuffer_cursorsSelectH (EditBuffer *editBuffer, int amount) {
        START_ALL_CURSORS_BATCH_OPERATION
                EditBuffer_Cursor_selectH(cursor, amount);
        END_ALL_CURSORS_BATCH_OPERATION
}

/* EditBuffer_cursorsSelectV
 * Extends the selection of all cursors vertically by amount
 */
void EditBuffer_cursorsSelectV (EditBuffer *editBuffer, int amount) {
        START_ALL_CURSORS_BATCH_OPERATION
                EditBuffer_Cursor_selectV(cursor, amount);
        END_ALL_CURSORS_BATCH_OPERATION
}

// TODO
void EditBuffer_cursorsSelectWordH (EditBuffer *, int);
void EditBuffer_cursorsSelectMoreV (EditBuffer *, int);
void EditBuffer_cursorsChangeIndent (EditBuffer *, int);
void EditBuffer_cursorsInsertString (EditBuffer *, String *);

/* EditBuffer_cursorsWrangle
 * Moves all cursors within bounds.
 */
static void EditBuffer_cursorsWrangle (EditBuffer *editBuffer) {
        START_ALL_CURSORS_BATCH_OPERATION
                EditBuffer_Cursor_wrangle(cursor);
        END_ALL_CURSORS_BATCH_OPERATION
        EditBuffer_mergeCursors(editBuffer);
}

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

        // make sure scroll is within bounds
        if (editBuffer->scroll >= editBuffer->length) {
                editBuffer->scroll = editBuffer->length;
        }
}

                  /* * * * * * * * * * * * * * * * * * * *
                   * EditBuffer_Cursor member functions  *
                   * * * * * * * * * * * * * * * * * * * */

/* EditBuffer_Cursor_insertRune
 * Inserts a character at the current cursor position. If there are no lines in
 * the edit buffer, this function does nothing.
 */
void EditBuffer_Cursor_insertRune (EditBuffer_Cursor *cursor, Rune rune) {
        if (cursor->parent->length == 0) { return; }
        EditBuffer_insertRuneAt (
                cursor->parent,
                cursor->column, cursor->row,
                rune);
}

/* EditBuffer_Cursor_deleteRune
 * Deletes the rune at the cursor. If there are no lines in the edit buffer,
 * this function does nothing.
 */
void EditBuffer_Cursor_deleteRune (EditBuffer_Cursor *cursor) {
        if (cursor->parent->length == 0) { return; }
        EditBuffer_deleteRuneAt(cursor->parent, cursor->column, cursor->row);
}

/* EditBuffer_Cursor_backspaceRune
 * Moves the cursor back once, and then deletes the rune at the cursor. If the
 * cursor cannot be moved back, this function does nothing.
 */
void EditBuffer_Cursor_backspaceRune (EditBuffer_Cursor *cursor) {
        if (cursor->column == 0 && cursor->row == 0) { return; }
        EditBuffer_Cursor_moveH(cursor, -1);
        EditBuffer_Cursor_deleteRune(cursor);
}

/* EditBuffer_Cursor_moveTo
 * Moves the cursor of the edit buffer to the specified row and column. This
 * resets the selection.
 */
void EditBuffer_Cursor_moveTo (
        EditBuffer_Cursor *cursor,
        size_t column,
        size_t row
) {
        cursor->hasSelection = 0;
        cursor->column = column;
        cursor->row    = row;
        EditBuffer_Cursor_selectNone(cursor);

        EditBuffer_mergeCursors(cursor->parent);
}

/* EditBuffer_Cursor_predictMovement
 * Returns the new position of the cursor, if it were to have moved from the
 * specified coordinates by the specified amount. It does not actually move the
 * cursor. This function is to be used as a common backend for other functions
 * that do change the cursor position, as it defines standard behavior for how
 * the cursor and selection should move in response to user input.
 */
static void EditBuffer_Cursor_predictMovement (
        EditBuffer_Cursor *cursor,
        size_t *resultColumn, size_t *resultRow,
        int amountH, int amountV
) {
        // predict row
        size_t rowBefore = *resultRow;
        *resultRow = constrainChange (
                *resultRow,
                amountV,
                cursor->parent->length);

        size_t lineLength = EditBuffer_getLine (
                cursor->parent,
                *resultRow)->length;
        if (*resultRow == rowBefore && amountV != 0) {
                // we wanted to move someplace, and did not get there. this
                // means we are at the end of the file, or the beginning of the
                // file, and we should snap the cursor to the beginning or end
                // of the line as expected
                if (amountV > 0) {
                        *resultColumn = lineLength;
                } else {
                        *resultColumn = 0;
                }
        }

        // ensure that the previous line jump did not send us into an invalid
        // spot
        if (*resultColumn > lineLength) {
                *resultColumn = lineLength;
        }

        // predict column
        if (*resultColumn == 0 && amountH < 0) {
                if (*resultRow > 0) {
                        *resultRow -= 1;
                        lineLength = EditBuffer_getLine (
                                cursor->parent, *resultRow
                        )->length;
                        *resultColumn = lineLength;
                }
                return;
        }

        // if we have gone off the end of the line, wrap around to the next one.
        lineLength = EditBuffer_getLine (
                cursor->parent,
                *resultRow)->length;
        if (*resultColumn >= lineLength && amountH > 0) {
                if (*resultColumn < cursor->parent->length - 1) {
                        *resultRow += 1;
                        *resultColumn = 0;
                }
                return;
        }
        
        *resultColumn = addToSizeT(*resultColumn, amountH);
}

/* EditBuffer_Cursor_moveH
 * Horizontally moves the cursor by amount. This function does bounds checking.
 * If the cursor runs off the line, it is taken to the previous or next line
 * (if possible),
 */
void EditBuffer_Cursor_moveH (EditBuffer_Cursor *cursor, int amount) {
        // if we have something selected, escape the selection and move to the
        // beginning or end depending on the direction of movement.
        if (cursor->hasSelection) {
                cursor->column = cursor->selectionColumn;
                cursor->row    = cursor->selectionRow;
                if (amount < 0) {
                        amount ++;
                }
        }

        EditBuffer_Cursor_predictMovement (
                cursor,
                &cursor->column, &cursor->row,
                amount, 0);

        EditBuffer_Cursor_selectNone(cursor);
        EditBuffer_mergeCursors(cursor->parent);
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
        // if we have something selected, escape the selection.
        // if (cursor->hasSelection) {
                // if (cursor->selectionDirection == EditBuffer_Direction_right) {
                        // cursor->column = cursor->endColumn + 1;
                // }
                // 
                // if (amount > 0) {
                        // cursor->row = cursor->endRow;
                // }
        // }
        
        EditBuffer_Cursor_predictMovement (
                cursor,
                &cursor->column, &cursor->row,
                0, amount);

        EditBuffer_Cursor_selectNone(cursor);
        EditBuffer_mergeCursors(cursor->parent);
}

// TODO
void EditBuffer_Cursor_moveWordH (EditBuffer_Cursor *cursor, int);
void EditBuffer_Cursor_moveMoreV (EditBuffer_Cursor *cursor, int);

/* EditBuffer_Cursor_selectH
 * Extends the selection of the cursor horizontally by amount. The selection end
 * must always come after the cursor origin - so this function will
 * automatically swap them if the given column and row are positioned before the
 * cursor origin.
 */
void EditBuffer_Cursor_selectH (EditBuffer_Cursor *cursor, int amount) {
        // if (!cursor->hasSelection) {
                // if (amount < 0) {
                        // cursor->selectionDirection = EditBuffer_Direction_left;
                        // EditBuffer_Cursor_moveH(cursor, -1);
                        // amount ++;
                // } else {
                        // cursor->selectionDirection = EditBuffer_Direction_right;
                        // amount --;
                // }
        // }
        // 
        // size_t newColumn;
        // size_t newRow;
// 
        // if (cursor->selectionDirection == EditBuffer_Direction_left) {
                // newColumn = cursor->column;
                // newRow    = cursor->row;
                // 
                // EditBuffer_Cursor_predictMovement (
                        // cursor,
                        // &newColumn, &newRow,
                        // amount, 0);
// 
                // // EditBuffer_Cursor_selectTo(cursor)
        // } else {
                // newColumn = cursor->endColumn;
                // newRow    = cursor->endRow;
        // }
// 
        // printf("%zu\t%zu\n", newColumn, newRow);
}

/* EditBuffer_Cursor_selectV
 * Extends the selection of the cursor vertically by amount. The selection end
 * must always come after the cursor origin - so this function will
 * automatically swap them if the given column and row are positioned before the
 * cursor origin.
 */
void EditBuffer_Cursor_selectV (EditBuffer_Cursor *cursor, int amount) {
        
}

// TODO
void EditBuffer_Cursor_selectWordH (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_selectMoreV (EditBuffer_Cursor *, int);

/* EditBuffer_Cursor_selectTo
 * Extends the selection to the specified coordinates.
 */
void EditBuffer_Cursor_selectTo (
        EditBuffer_Cursor *cursor,
        size_t column, size_t row
) {
        cursor->hasSelection = 1;

        cursor->selectionColumn = column;
        cursor->selectionRow    = row;
}

/* EditBuffer_Cursor_selectNone
 * Clears the selection of the cursor. 
 */
void EditBuffer_Cursor_selectNone (EditBuffer_Cursor *cursor) {
        cursor->hasSelection = 0;

        cursor->selectionColumn = cursor->column;
        cursor->selectionRow    = cursor->row;
}

// TODO
void EditBuffer_Cursor_changeIndent (EditBuffer_Cursor *cursor, int);
void EditBuffer_Cursor_insertString (EditBuffer_Cursor *cursor, String *string);

/* EditBuffer_Cursor_wrangle
 * Moves cursor within bounds. If there are no lines in the edit buffer, this
 * function does nothing.
 */
static void EditBuffer_Cursor_wrangle (EditBuffer_Cursor *cursor) {
        if (cursor->parent->length == 0) { return; }
        String *line;
        
        if (cursor->row >= cursor->parent->length) {
                cursor->row = cursor->parent->length - 1;
                line = EditBuffer_Cursor_getCurrentLine(cursor);
                cursor->column = line->length;
                return;
        }
        
        line = EditBuffer_Cursor_getCurrentLine(cursor);

        if (cursor->column > line->length) {
                cursor->column = line->length;
        }
}

/* EditBuffer_Cursor_getCurrentLine
 * Returns a pointer to the line that the cursor is currently on.
 */
String *EditBuffer_Cursor_getCurrentLine (EditBuffer_Cursor *cursor) {
        return EditBuffer_getLine(cursor->parent, cursor->row);
}

/* EditBuffer_Cursor_getSelectionBounds
 * Gets the sorted selection boundary of the cursor.
 */
void EditBuffer_Cursor_getSelectionBounds (
        EditBuffer_Cursor *cursor,
        size_t *startRow, size_t *startColumn,
        size_t *endRow,   size_t *endColumn
) {
        int rowOutOfOrder    = cursor->selectionRow    < cursor->row;
        int onSameRow        = cursor->selectionRow   == cursor->row;
        int columnOutOfOrder = cursor->selectionColumn < cursor->column;

        if (rowOutOfOrder || (columnOutOfOrder && onSameRow)) {
                *startRow    = cursor->selectionRow;
                *startColumn = cursor->selectionColumn;
                *endRow      = cursor->row;
                *endColumn   = cursor->column;
        } else {
                *startRow    = cursor->row;
                *startColumn = cursor->column;
                *endRow      = cursor->selectionRow;
                *endColumn   = cursor->selectionColumn;
        }
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

/* addToSizeT
 * Behaves the same as constrainChange, but does not specify an upper bound.
 */
static size_t addToSizeT (size_t initial, int amount) {
        size_t amountAbs;
        
        if (amount < 0) {
                amountAbs = (size_t)(0 - amount);
                return initial - amountAbs;
        } else {
                amountAbs = (size_t)(amount);
                return initial + amountAbs;
        }
}
