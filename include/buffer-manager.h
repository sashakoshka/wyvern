#pragma once

#include "edit-buffer.h"

void        BufferManager_init   (void);
size_t      BufferManager_add    (EditBuffer *);
size_t      BufferManager_addNew (void);
Error       BufferManager_delete (size_t);
EditBuffer *BufferManager_get    (size_t);
