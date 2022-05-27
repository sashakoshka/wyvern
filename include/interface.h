#pragma once

#include "safe-string.h"
#include "edit-buffer.h"
#include "error.h"

Error Interface_run           (void);
void  Interface_setEditBuffer (EditBuffer *newEditBuffer);

// make event handlers for creating a new tab, clicking on a tab, etc
