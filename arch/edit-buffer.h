#pragma once

#include <stdlib.h>
#include "safe-string.h"
#include "error.h"

typedef struct EditBuffer_Line {
        String *string;
        struct EditBuffer_Line *prev;
        struct EditBuffer_Line *next;
} EditBuffer_Line;

typedef struct {
        EditBuffer_Line *lines;
        EditBuffer_Line *currentLine;
        EditBuffer_Line *scrollLine;
        
        size_t column;
} EditBuffer;

EditBuffer *EditBuffer_new  (void);
void        EditBuffer_free (EditBuffer *);

Error EditBuffer_open  (EditBuffer *, const char *);
void  EditBuffer_copy  (EditBuffer *, const char *);
void  EditBuffer_clear (EditBuffer *editBuffer);

void EditBuffer_insertChar (EditBuffer *, char);
void EditBuffer_deleteChar (EditBuffer *);

void EditBuffer_cursorMoveH     (EditBuffer *, int);
void EditBuffer_cursorMoveV     (EditBuffer *, int);
void EditBuffer_cursorMoveWordH (EditBuffer *, int);
void EditBuffer_cursorMoveWordV (EditBuffer *, int);

void EditBuffer_changeIndent (EditBuffer *, int);
void EditBuffer_insertBuffer (EditBuffer *, const char *);
