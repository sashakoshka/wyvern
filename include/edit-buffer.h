#pragma once

#include <stdlib.h>
#include <limits.h>
#include "safe-string.h"
#include "error.h"

typedef struct EditBuffer_Cursor EditBuffer_Cursor;
typedef struct EditBuffer        EditBuffer;

struct EditBuffer_Cursor {
        size_t row;
        size_t column;

        EditBuffer *parent;
};

struct EditBuffer {
        EditBuffer_Cursor cursor;
        
        size_t scroll;
        
        size_t length;
        size_t size;
        String **lines;

        char filePath[PATH_MAX];
};

EditBuffer *EditBuffer_new  (void);
void        EditBuffer_free (EditBuffer *);

Error EditBuffer_open  (EditBuffer *, const char *);
void  EditBuffer_copy  (EditBuffer *, const char *);
void  EditBuffer_reset (EditBuffer *editBuffer);

void EditBuffer_scroll (EditBuffer *, int);

String *EditBuffer_getLine (EditBuffer *, size_t);

void EditBuffer_insertRune         (EditBuffer *, Rune);
void EditBuffer_deleteRune         (EditBuffer *);
void EditBuffer_cursorMoveH        (EditBuffer *, int);
void EditBuffer_cursorMoveV        (EditBuffer *, int);
void EditBuffer_cursorMoveWordH    (EditBuffer *, int);
void EditBuffer_cursorMoveWordV    (EditBuffer *, int);
void EditBuffer_cursorMoveTo       (EditBuffer *, size_t, size_t);
void EditBuffer_cursorChangeIndent (EditBuffer *, int);
void EditBuffer_cursorInsertBuffer (EditBuffer *, const char *);

// TODO: create methods for backspacing, overwriting, and inserting + moving 1
// forward
void EditBuffer_Cursor_insertRune        (EditBuffer_Cursor *, Rune);
void EditBuffer_Cursor_deleteRune        (EditBuffer_Cursor *);
void EditBuffer_Cursor_moveH             (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_moveV             (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_moveWordH         (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_moveWordV         (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_moveTo            (EditBuffer_Cursor *, size_t, size_t);
void EditBuffer_Cursor_changeIndent      (EditBuffer_Cursor *, int);
void EditBuffer_Cursor_insertString      (EditBuffer_Cursor *, String *);
String *EditBuffer_Cursor_getCurrentLine (EditBuffer_Cursor *);
