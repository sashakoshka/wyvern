#include <stdio.h>
#include "edit-buffer.h"

EditBuffer_Line *EditBuffer_newLine   (EditBuffer *, EditBuffer_Line *, int);
void             EditBuffer_liftLine  (EditBuffer *, EditBuffer_Line *line);
void             EditBuffer_placeLine (
        EditBuffer *,
        EditBuffer_Line *,
        EditBuffer_Line *,
        int);

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

        FILE *file = fopen(filePath, "r");
        if (file == NULL) { return Error_cantOpenFile; }

        EditBuffer_Line *line = NULL;
        int ch = '\n';
        while (ch != EOF) {
                if (ch == '\n') {
                        line = EditBuffer_newLine(editBuffer, line, 1);
                } else {
                        String_addChar(line->string, (char)ch);
                }
                ch = fgetc(file);
        }
        
        editBuffer->currentLine = editBuffer->lines;
        editBuffer->scrollLine  = editBuffer->lines;

        fclose(file);
        return Error_cantOpenFile;
}

/* EditBuffer_copy
 * Copies in a char buffer into an edit buffer.
 */
void EditBuffer_copy (EditBuffer *editBuffer, const char *buffer) {
        EditBuffer_clear(editBuffer);

        EditBuffer_Line *line = EditBuffer_newLine(editBuffer, NULL, 1);
        int ch;
        for (size_t index = 0; (ch = buffer[index]); index ++) {
                if (ch == '\n') {
                        line = EditBuffer_newLine(editBuffer, line, 1);
                } else {
                        String_addChar(line->string, (char)ch);
                }
        }
}

/* EditBuffer_clear
 * Resets the buffer, clearing and freeing its contents. After this function,
 * the buffer itself can be safely freed, or new content can be loaded or
 * entered in.
 */
void EditBuffer_clear (EditBuffer *editBuffer) {
        EditBuffer_Line *line = editBuffer->lines;

        while (line != NULL) {
                EditBuffer_Line *next = line->next;
                String_free(line->string);
                free(line);
                line = next;
        }

        *editBuffer = (const EditBuffer) { 0 };
}

/* EditBuffer_insertChar
 * Inserts a character at the current cursor position. If there are no lines in
 * the edit buffer, this function does nothing.
 */
void EditBuffer_insertChar (EditBuffer *editBuffer, char ch) {
        if (editBuffer->currentLine == NULL) { return; }

        if (ch == '\n') {
                // TODO: split line in two
                return;
        }

        String_insertChar (
                editBuffer->currentLine->string,
                ch, editBuffer->column);
}

/* EditBuffer_deleteChar
 * Deletes the char at the cursor.
 */
void EditBuffer_deleteChar (EditBuffer *editBuffer) {
        if (editBuffer->currentLine == NULL) { return; }
        String_deleteChar(editBuffer->currentLine->string, editBuffer->column);
}

void EditBuffer_cursorMoveH (EditBuffer *editBuffer, int);
void EditBuffer_cursorMoveV (EditBuffer *editBuffer, int);
void EditBuffer_cursorMoveWordH (EditBuffer *editBuffer, int);
void EditBuffer_cursorMoveWordV (EditBuffer *editBuffer, int);

void EditBuffer_changeIndent (EditBuffer *editBuffer, int);
void EditBuffer_insertBuffer (EditBuffer *editBuffer, const char *);

/* EditBuffer_insertLine
 * Inserts a blank line in place of the specified already existing line, moving
 * that line and the rest of the lines downwards. Returns a pointer to the newly
 * created blank line. If after is 1, the new line will be inserted after the
 * insertion point instead. If insertionPoint is null, the line will be inserted
 * at the beginning.
 */
EditBuffer_Line *EditBuffer_newLine (
        EditBuffer      *editBuffer,
        EditBuffer_Line *insertionPoint,
        int after
) {
        EditBuffer_Line *line = calloc(1, sizeof(EditBuffer_Line));
        line->string = String_new("");
        EditBuffer_placeLine(editBuffer, line, insertionPoint, after);
        return line;
}

/* EditBuffer_liftLine
 * Lifts a line out of its edit buffer, preserving it.
 */
void EditBuffer_liftLine (EditBuffer *editBuffer, EditBuffer_Line *line) {
        if (line->prev == NULL) {
                editBuffer->lines = line->next;
        } else {
                line->prev->next = line->next;
        }

        if (line->next != NULL) {
                line->next->prev = line->prev;
        }

        line->prev = NULL;
        line->next = NULL;
}

/* EditBuffer_placeLine
 * Inserts a line in place of the specified already existing line, moving that
 * line and the rest of the lines downwards. If after is 1, the line will be
 * inserted after the insertion point instead. If insertionPoint is null, the
 * line will be inserted at the beginning.
 */
void EditBuffer_placeLine (
        EditBuffer      *editBuffer,
        EditBuffer_Line *line,
        EditBuffer_Line *insertionPoint,
        int after
) {
        if (insertionPoint == NULL) {
                line->next = editBuffer->lines;
                editBuffer->lines = line;
                return;
        }

        if (after) {
                EditBuffer_Line *next = insertionPoint->next;

                insertionPoint->next = line;
                line->prev = insertionPoint;

                if (next != NULL) {
                        next->prev = line;
                }
                line->next = next;
        } else {
                EditBuffer_Line *prev = insertionPoint->prev;

                insertionPoint->prev = line;
                line->next = insertionPoint;

                if (prev == NULL) {
                        editBuffer->lines = line;
                } else {
                        prev->next = line;
                }

                line->prev = prev;
        }

}
