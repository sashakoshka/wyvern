#pragma once

#include <stdlib.h>
#include <limits.h>
#include "safe-string.h"
#include "error.h"

typedef struct {
        size_t row;
        size_t column;
        size_t scroll;
        
        size_t length;
        size_t size;
        String **lines;

        char filePath[PATH_MAX];
} EditBuffer;

EditBuffer *EditBuffer_new  (void);
void        EditBuffer_free (EditBuffer *);

Error EditBuffer_open  (EditBuffer *, const char *);
void  EditBuffer_copy  (EditBuffer *, const char *);
void  EditBuffer_clear (EditBuffer *editBuffer);

void EditBuffer_insertRune (EditBuffer *, Rune);
void EditBuffer_deleteRune (EditBuffer *);

void EditBuffer_scroll (EditBuffer *, int);

void EditBuffer_cursorMoveH     (EditBuffer *, int);
void EditBuffer_cursorMoveV     (EditBuffer *, int);
void EditBuffer_cursorMoveWordH (EditBuffer *, int);
void EditBuffer_cursorMoveWordV (EditBuffer *, int);
void EditBuffer_cursorMoveTo    (EditBuffer *, size_t, size_t);

void EditBuffer_changeIndent (EditBuffer *, int);
void EditBuffer_insertBuffer (EditBuffer *, const char *);
