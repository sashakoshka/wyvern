#pragma once

#include <stdlib.h>
#include <limits.h>
#include "safe-string.h"
#include "error.h"

typedef struct EditBuffer_Cursor EditBuffer_Cursor;
typedef struct EditBuffer        EditBuffer;

#define EDITBUFFER_MAX_CURSORS 32

struct EditBuffer_Cursor {
        size_t row;
        size_t column;
        
        size_t endRow;
        size_t endColumn;

        EditBuffer *parent;
        
        int hasSelection;
};

struct EditBuffer {
        EditBuffer_Cursor cursors[EDITBUFFER_MAX_CURSORS];
        size_t            amountOfCursors;
        
        size_t scroll;
        
        size_t length;
        size_t size;
        String **lines;

        int dontMerge;

        char filePath[PATH_MAX];
};

EditBuffer *EditBuffer_new  (void);
void        EditBuffer_free (EditBuffer *);

Error EditBuffer_open              (EditBuffer *, const char *);
void  EditBuffer_copy              (EditBuffer *, const char *);
void  EditBuffer_reset             (EditBuffer *);
void  EditBuffer_clearExtraCursors (EditBuffer *);
void  EditBuffer_addNewCursor      (EditBuffer *, size_t, size_t);
int   EditBuffer_hasCursorAt       (EditBuffer *, size_t, size_t);
int   EditBuffer_hasSelectionAt    (EditBuffer *, size_t, size_t);
void  EditBuffer_insertRuneAt      (EditBuffer *, size_t, size_t, Rune);
void  EditBuffer_deleteRuneAt      (EditBuffer *, size_t, size_t);

void EditBuffer_scroll (EditBuffer *, int);

String *EditBuffer_getLine (EditBuffer *, size_t);

void EditBuffer_cursorsInsertRune    (EditBuffer *, Rune);
void EditBuffer_cursorsDeleteRune    (EditBuffer *);
void EditBuffer_cursorsBackspaceRune (EditBuffer *);
void EditBuffer_cursorsMoveH         (EditBuffer *, int);
void EditBuffer_cursorsMoveV         (EditBuffer *, int);
void EditBuffer_cursorsMoveWordH     (EditBuffer *, int);
void EditBuffer_cursorsMoveMoreV     (EditBuffer *, int);
void EditBuffer_cursorsSelectH       (EditBuffer *, int);
void EditBuffer_cursorsSelectV       (EditBuffer *, int);
void EditBuffer_cursorsSelectWordH   (EditBuffer *, int);
void EditBuffer_cursorsSelectMoreV   (EditBuffer *, int);
void EditBuffer_cursorsChangeIndent  (EditBuffer *, int);
void EditBuffer_cursorsInsertString  (EditBuffer *, String *);

// TODO: create methods for backspacing, overwriting, and inserting + moving 1
// forward
void EditBuffer_Cursor_insertRune        (EditBuffer_Cursor *, Rune);
void EditBuffer_Cursor_deleteRune        (EditBuffer_Cursor *);
void EditBuffer_Cursor_backspaceRune     (EditBuffer_Cursor *);
void EditBuffer_Cursor_moveTo            (EditBuffer_Cursor *, size_t, size_t);
void EditBuffer_Cursor_moveH             (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_moveV             (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_moveWordH         (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_moveMoreV         (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_selectTo          (EditBuffer_Cursor *, size_t, size_t);
void EditBuffer_Cursor_selectH           (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_selectV           (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_selectWordH       (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_selectMoreV       (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_changeIndent      (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_insertString      (EditBuffer_Cursor *, String *);
String *EditBuffer_Cursor_getCurrentLine (EditBuffer_Cursor *);
