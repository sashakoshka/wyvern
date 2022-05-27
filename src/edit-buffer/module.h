#include <stdio.h>
#include <string.h>

#include "edit-buffer.h"
#include "options.h"
#include "utility.h"

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

void EditBuffer_placeLine      (EditBuffer *, String *, size_t);
void EditBuffer_realloc        (EditBuffer *, size_t);
void EditBuffer_shiftDown      (EditBuffer *, size_t, size_t);
void EditBuffer_shiftUp        (EditBuffer *, size_t, size_t, int);
void EditBuffer_shiftCursorsInLineAfter (
        EditBuffer *,
        size_t, size_t,
        int);
void EditBuffer_mergeCursors   (EditBuffer *);
void EditBuffer_cursorsWrangle (EditBuffer *);
void EditBuffer_Cursor_wrangle (EditBuffer_Cursor *);

void EditBuffer_Cursor_predictMovement (
        EditBuffer_Cursor *,
        size_t *, size_t *,
        int, int);
