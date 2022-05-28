#pragma once

#include "safe-string.h"
#include "edit-buffer.h"
#include "error.h"

typedef struct Interface_Tab Interface_Tab;

Error Interface_run           (void);
void  Interface_setEditBuffer (EditBuffer *newEditBuffer);

Interface_Tab *Interface_TabBar_add    (void);
void           Interface_TabBar_delete (Interface_Tab *);

// make event handlers for creating a new tab, clicking on a tab, etc
