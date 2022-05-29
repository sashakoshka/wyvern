#pragma once

#include "safe-string.h"
#include "edit-buffer.h"
#include "error.h"

typedef struct Interface_Tab      Interface_Tab;
typedef struct Interface_Object   Interface_Object;
typedef struct Interface_TabBar   Interface_TabBar;
typedef struct Interface_EditView Interface_EditView;
typedef struct Interface          Interface;

Error Interface_run           (void);
void  Interface_setEditBuffer (EditBuffer *newEditBuffer);

Interface_Tab *Interface_tabBar_add       (void);
void           Interface_tabBar_delete    (Interface_Tab *);
void           Interface_tabBar_setActive (Interface_Tab *);
void           Interface_Tab_setText      (Interface_Tab *, const char *);

void Interface_onStart     (void (*) (void));
void Interface_onNewTab    (void (*) (void));
void Interface_onCloseTab  (void (*) (Interface_Tab *));
void Interface_onSwitchTab (void (*) (Interface_Tab *));

// make event handlers for creating a new tab, clicking on a tab, etc
