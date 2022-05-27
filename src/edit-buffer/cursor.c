#include "module.h"

/* EditBuffer_Cursor_insertRune
 * Inserts a character at the current cursor position. If there are no lines in
 * the edit buffer, this function does nothing.
 */
void EditBuffer_Cursor_insertRune (EditBuffer_Cursor *cursor, Rune rune) {
        if (cursor->parent->length == 0) { return; }
        if (cursor->hasSelection) {
                EditBuffer_Cursor_deleteSelection(cursor);
        }
        EditBuffer_insertRuneAt (
                cursor->parent,
                cursor->column, cursor->row,
                rune);
}

/* EditBuffer_Cursor_deleteSelection
 * Deletes all text in the selection of the cursor.
 */
void EditBuffer_Cursor_deleteSelection (EditBuffer_Cursor *cursor) {
        size_t startColumn;
        size_t startRow;
        size_t endColumn;
        size_t endRow;

        EditBuffer_Cursor_getSelectionBounds (
                cursor,
                &startColumn, &startRow,
                &endColumn,   &endRow);

        EditBuffer_deleteRange (
                cursor->parent,
                startColumn, startRow,
                endColumn, endRow);

        EditBuffer_Cursor_selectNone(cursor);
}

/* EditBuffer_Cursor_deleteRune
 * Deletes the rune at the cursor. If there are no lines in the edit buffer,
 * this function does nothing.
 */
void EditBuffer_Cursor_deleteRune (EditBuffer_Cursor *cursor) {
        if (cursor->parent->length == 0) { return; }
        if (cursor->hasSelection) {
                EditBuffer_Cursor_deleteSelection(cursor);
        } else {
                EditBuffer_deleteRuneAt (
                        cursor->parent,
                        cursor->column, cursor->row);
        }
}

/* EditBuffer_Cursor_backspaceRune
 * Moves the cursor back once, and then deletes the rune at the cursor. If the
 * cursor cannot be moved back, this function does nothing.
 */
void EditBuffer_Cursor_backspaceRune (EditBuffer_Cursor *cursor) {
        if (cursor->column == 0 && cursor->row == 0) { return; }
        if (cursor->hasSelection) {
                EditBuffer_Cursor_deleteSelection(cursor);
        } else {
                EditBuffer_Cursor_moveH(cursor, -1);
                EditBuffer_Cursor_deleteRune(cursor);
        }
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
void EditBuffer_Cursor_predictMovement (
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
                if (*resultRow < cursor->parent->length - 1) {
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
                size_t startColumn;
                size_t startRow;
                size_t endColumn;
                size_t endRow;

                EditBuffer_Cursor_getSelectionBounds (
                        cursor,
                        &startColumn, &startRow,
                        &endColumn,   &endRow);
                
                if (amount < 0) {
                        cursor->column = startColumn;
                        cursor->row    = startRow;
                        amount ++;
                } else {
                        cursor->column = endColumn;
                        cursor->row    = endRow;
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
        if (cursor->hasSelection) {
                if (cursor->row == cursor->selectionRow) {
                        // if the selection is only on one row, then escape
                        // the selection directionally
                        int escapingToRight =
                                cursor->selectionColumn > cursor->column;
                        
                        cursor->column = cursor->selectionColumn;
                        cursor->row    = cursor->selectionRow;

                        if (escapingToRight) {
                                cursor->column ++;
                        }
                } else {
                        size_t startColumn;
                        size_t startRow;
                        size_t endColumn;
                        size_t endRow;

                        EditBuffer_Cursor_getSelectionBounds (
                                cursor,
                                &startColumn, &startRow,
                                &endColumn,   &endRow);
                        
                        if (amount < 0) {
                                cursor->column = startColumn;
                                cursor->row    = startRow;
                        } else {
                                cursor->column = endColumn + 1;
                                cursor->row    = endRow;
                        }
                }
                
        }
        
        EditBuffer_Cursor_predictMovement (
                cursor,
                &cursor->column, &cursor->row,
                0, amount);

        EditBuffer_Cursor_selectNone(cursor);
        EditBuffer_mergeCursors(cursor->parent);
}

// TODO
// TODO: remove amount inputs on these functions, and have them use the other
// functions but have a function in the string that gets the position of the
// next word.
void EditBuffer_Cursor_moveWordH (EditBuffer_Cursor *cursor, int);
void EditBuffer_Cursor_moveMoreV (EditBuffer_Cursor *cursor, int);

/* EditBuffer_Cursor_selectH
 * Extends the selection of the cursor horizontally by amount. The selection end
 * must always come after the cursor origin - so this function will
 * automatically swap them if the given column and row are positioned before the
 * cursor origin.
 */
void EditBuffer_Cursor_selectH (EditBuffer_Cursor *cursor, int amount) {
        // if selection is new, and direction is to the right, set selection to
        // cursor position and exit.
        if (amount > 0 && !cursor->hasSelection) {
                cursor->hasSelection = 1;
                EditBuffer_Cursor_selectTo(cursor, cursor->column, cursor->row);
                return;
        }

        // otherwise, extend.
        size_t newColumn = cursor->selectionColumn;
        size_t newRow    = cursor->selectionRow;
        
        EditBuffer_Cursor_predictMovement (
                cursor,
                &newColumn, &newRow,
                amount, 0);
        EditBuffer_Cursor_selectTo(cursor, newColumn, newRow);
}

/* EditBuffer_Cursor_selectV
 * Extends the selection of the cursor vertically by amount. The selection end
 * must always come after the cursor origin - so this function will
 * automatically swap them if the given column and row are positioned before the
 * cursor origin.
 */
void EditBuffer_Cursor_selectV (EditBuffer_Cursor *cursor, int amount) {
        int amountH = 0;

        // when starting out, we need to move back one horizontally.
        if (amount > 0 && !cursor->hasSelection) {
                amountH = -1;
        }

        size_t newColumn = cursor->selectionColumn;
        size_t newRow    = cursor->selectionRow;
        
        EditBuffer_Cursor_predictMovement (
                cursor,
                &newColumn, &newRow,
                amountH, amount);
        EditBuffer_Cursor_selectTo(cursor, newColumn, newRow);
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
void EditBuffer_Cursor_wrangle (EditBuffer_Cursor *cursor) {
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
        size_t *startColumn, size_t *startRow,
        size_t *endColumn,   size_t *endRow
) {
        int rowOutOfOrder    = cursor->selectionRow    < cursor->row;
        int onSameRow        = cursor->selectionRow   == cursor->row;
        int columnOutOfOrder = cursor->selectionColumn < cursor->column;

        if (rowOutOfOrder || (columnOutOfOrder && onSameRow)) {
                // out of order (seelction is behind cursor)
                *startRow    = cursor->selectionRow;
                *startColumn = cursor->selectionColumn;
                *endRow      = cursor->row;
                *endColumn   = cursor->column - 1;
        } else {
                // in order (selection is in front of cursor)
                *startRow    = cursor->row;
                *startColumn = cursor->column;
                *endRow      = cursor->selectionRow;
                *endColumn   = cursor->selectionColumn;
        }
}
