#include "module.h"
#include "options.h"

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
        Utility_copyCString(editBuffer->filePath, filePath, PATH_MAX);

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
        START_ALL_CURSORS
                if (
                        !cursor->hasSelection    &&
                        cursor->column == column &&
                        cursor->row    == row
                ) {
                        return 1;
                }
        END_ALL_CURSORS

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
                size_t startColumn;
                size_t startRow;
                size_t endColumn;
                size_t endRow;

                EditBuffer_Cursor_getSelectionBounds (
                        cursor,
                        &startColumn, &startRow,
                        &endColumn,   &endRow);

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

                // TODO: possibly combine these two into one loop
                
                // shift down cursors after the new line break
                START_ALL_CURSORS
                        if (cursor->row < row) {
                                // this cursor comes before the insertion
                                continue;
                        }

                        if (cursor->row > row) {
                                // this cursor is in a row after the insertion,
                                // so it simply needs to be shifted down.
                                cursor->row ++;
                                continue;
                        }

                        // these are on the same column as the insertion

                        if (cursor->column < column) {
                                // this cursor comes before the insertion
                                continue;
                        }
                
                        if (cursor->column == column) {
                                // this is the cursor that caused the
                                // insertion, so it should wrap around
                                // to the beginning of the next line
                                cursor->column = 0;
                                cursor->row ++;
                                continue;
                        }
                
                        if (cursor->column > column) {
                                // this cursor was previously on the
                                // part of the line that got split and
                                // made into its own line. it needs to
                                // have its position set to the proper
                                // place on that new line.
                                cursor->column -= currentLine->length;
                                cursor->row ++;
                                continue;
                        }
                END_ALL_CURSORS

                // the same thing, but for the selection coordinates
                START_ALL_CURSORS
                        if (cursor->selectionRow < row) {  continue; }

                        if (cursor->selectionRow > row) {
                                cursor->selectionRow ++;
                                continue;
                        }

                        if (cursor->selectionColumn < column) { continue; }
                
                        if (cursor->selectionColumn == column) {
                                cursor->selectionColumn = 0;
                                cursor->selectionRow ++;
                                continue;
                        }
                
                        if (cursor->selectionColumn > column) {
                                cursor->selectionColumn -= currentLine->length;
                                cursor->selectionRow ++;
                                continue;
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

                // do the same thing with the selection end
                if (cursor->selectionRow > row) {
                        cursor->selectionRow --;
                        if (cursor->selectionRow == row) {
                                cursor->selectionColumn += previousLength;
                        }
                }
        END_ALL_CURSORS

        // since we deleted a line, there might be cursors that are now out of
        // bounds, and we need to bring them back in.
        EditBuffer_cursorsWrangle(editBuffer);
}

/* EditBuffer_deleteRange
 * Deletes all runes in the specified range, inclusive.
 */
void  EditBuffer_deleteRange (
        EditBuffer *editBuffer,
        size_t startColumn, size_t startRow,
        size_t endColumn,   size_t endRow
) {
        size_t numberOfLines = endRow - startRow + 1;
        
        if (numberOfLines >= 3) {
                // there are lines in the middle we can quickly deal with
                size_t numberOfMiddleLines = numberOfLines - 2;
                EditBuffer_shiftUp (
                        editBuffer,
                        startRow + 1, numberOfMiddleLines, 0);

                START_ALL_CURSORS
                        if (cursor->row > startRow) {
                                cursor->row -= numberOfMiddleLines;
                        }
                        
                        if (cursor->selectionRow > startRow) {
                                cursor->selectionRow -= numberOfMiddleLines;
                        }
                END_ALL_CURSORS

                endRow -= numberOfMiddleLines;

                // since we deleted lines, there might be cursors that are now
                // out of bounds, and we need to bring them back in.
                EditBuffer_cursorsWrangle(editBuffer);
        }
        
        numberOfLines = endRow - startRow + 1;

        if (numberOfLines >= 2) {
                // we have a start and end line
                String *line = EditBuffer_getLine(editBuffer, startRow);
                size_t toDelete = line->length - startColumn + endColumn + 2;
                
                for (size_t index = 0; index < toDelete; index ++) {
                        EditBuffer_deleteRuneAt (
                                editBuffer,
                                startColumn, startRow);
                }
        } else {
                // we have a line
                for (size_t index = startColumn; index <= endColumn; index ++) {
                        EditBuffer_deleteRuneAt (
                                editBuffer,
                                startColumn, startRow);
                }
        }
}

/* EditBuffer_shiftCursorsInLineAfter
 * Shifts all cursors in row that are positioned at or equal to column by
 * amount. This function should ONLY be used in rune insertion and deletion, and
 * not as a replacement for EditBuffer_Cursor_moveH.
 */
void EditBuffer_shiftCursorsInLineAfter (
        EditBuffer *editBuffer,
        size_t column, size_t row,
        int amount
) {
        START_ALL_CURSORS
                // shift cursor coordinates
                if (
                        cursor->row == row &&
                        cursor->column >= column
                ) {
                        cursor->column = Utility_addToSizeT (
                                cursor->column,
                                amount);

                }

                // shift selection coordinates
                if (
                        cursor->selectionRow == row &&
                        cursor->selectionColumn >= column
                ) {
                        cursor->selectionColumn = Utility_addToSizeT (
                                cursor->selectionColumn,
                                amount);
                }
        END_ALL_CURSORS
}

/* EditBuffer_mergeCursors
 * Removes redundant, overlapping cursors. This should be called whenever a
 * cursor moves.
 */
void EditBuffer_mergeCursors (EditBuffer *editBuffer) {
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
        editBuffer->scroll = Utility_constrainChange (
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
void EditBuffer_placeLine (
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
void EditBuffer_shiftDown (
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
void EditBuffer_shiftUp (
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
}

/* EditBuffer_cursorsInsertRune
 * Inserts a rune at all cursors.
 */
void EditBuffer_cursorsInsertRune (EditBuffer *editBuffer, Rune rune) {
        START_ALL_CURSORS_BATCH_OPERATION
                EditBuffer_Cursor_insertRune(cursor, rune);
        END_ALL_CURSORS_BATCH_OPERATION
}

/* EditBuffer_cursorsDeleteSelection
 * Deletes all text in the selection of all cursors.
 */
void EditBuffer_cursorsDeleteSelection (EditBuffer *editBuffer) {
        START_ALL_CURSORS_BATCH_OPERATION
                EditBuffer_Cursor_deleteSelection(cursor);
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
void EditBuffer_cursorsWrangle (EditBuffer *editBuffer) {
        START_ALL_CURSORS_BATCH_OPERATION
                EditBuffer_Cursor_wrangle(cursor);
        END_ALL_CURSORS_BATCH_OPERATION
        EditBuffer_mergeCursors(editBuffer);
}

/* EditBuffer_realloc
 * Resizes the internal buffer of the edit buffer to accomodate a file of
 * newLength lines.
 */
void EditBuffer_realloc (EditBuffer *editBuffer, size_t newLength) {
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
