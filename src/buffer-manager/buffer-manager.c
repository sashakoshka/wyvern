#include "buffer-manager.h"

static struct {
        EditBuffer **list;
        size_t size;
} store = { 0 };

void BufferManager_init (void) {
        store.list = calloc(1, sizeof(store.list[0]));
}

size_t BufferManager_add (EditBuffer *editBuffer) {
        for (size_t index = 0; index < store.size; index ++) {
                if (store.list[index] == NULL) {
                        store.list[index] = editBuffer;
                        return index;
                }
        }

        store.size ++;
        store.list = realloc (
                store.list,
                sizeof(store.list[0]) * store.size );

        store.list[store.size - 1] = editBuffer;
        return store.size - 1;
}

size_t BufferManager_addNew (void) {
        return BufferManager_add(EditBuffer_new()); 
}

Error BufferManager_delete (size_t index) {
        if (index >= store.size) { return Error_outOfBounds; }
        EditBuffer_free(store.list[index]);
        store.list[index] = NULL;
        
        if (index == store.size - 1) {
                store.size --;
                store.list = realloc (
                        store.list,
                        sizeof(store.list[0]) * store.size );
        }
        
        return Error_none;
}

EditBuffer *BufferManager_get (size_t index) {
        if (index >= store.size) { return NULL; }
        return store.list[index];
}
