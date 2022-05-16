#include <stdio.h>
#include "edit-buffer.h"

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

        String *line = line = String_new("");
        int  reachedEnd = 0;
        while (!reachedEnd) {
                Rune rune = Unicode_utf8FileGetRune(file, &reachedEnd);
                if (rune == 0) { continue; }
                
                if (rune == '\n') {
                        EditBuffer_placeLine (
                                editBuffer, line,
                                editBuffer->length);
                        line = String_new("");
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

        String *line = line = String_new("");
        int ch;
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
                // TODO: split line in two
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
        String_deleteRune (
                editBuffer->lines[editBuffer->row],
                editBuffer->column);
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

void EditBuffer_cursorMoveH (EditBuffer *editBuffer, int amount);

/* EditBuffer_cursorMoveH
 * Vertically moves the cursor by amount. This function does bounds checking.
 */
void EditBuffer_cursorMoveV (EditBuffer *editBuffer, int amount) {
        editBuffer->scroll = constrainChange (
                editBuffer->row,
                amount,
                editBuffer->length);
}

void EditBuffer_cursorMoveWordH (EditBuffer *editBuffer, int);
void EditBuffer_cursorMoveWordV (EditBuffer *editBuffer, int);

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
        EditBuffer_realloc(editBuffer, editBuffer->length + amount);

        size_t current = editBuffer->length - amount;
        while (current --> index) {
                editBuffer->lines[current] =
                        editBuffer->lines[current + amount];
                editBuffer->lines[current] = NULL;
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
        size_t end = index + amount;
        for (; index < end; index ++) {
                if (!keep) { String_free(editBuffer->lines[index]); }
                editBuffer->lines[index] = editBuffer->lines[index + amount];
        }
        
        EditBuffer_realloc(editBuffer, editBuffer->length - amount);
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
                editBuffer->size * sizeof(editBuffer->lines));
        editBuffer->length = newLength;

        // make sure cursor parameters are within bounds
        if (editBuffer->scroll >= editBuffer->length) {
                editBuffer->scroll = editBuffer->length;
        }

        if (editBuffer->row >= editBuffer->length) {
                editBuffer->row = editBuffer->length;
        }
}

/* constrain
 * Constrains a change in initial of amount, to a lower bound of 0 and an upper
 * bound of bound (bound is not inclusive).
 */
static size_t constrainChange (size_t initial, int amount, size_t bound) {
        size_t amountAbs;
        
        if (amount < 0) {
                amountAbs = 0 - (size_t)amount;
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
