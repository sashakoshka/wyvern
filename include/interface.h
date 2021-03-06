#pragma once

#include "safe-string.h"
#include "edit-buffer.h"
#include "error.h"

typedef struct Interface_Object         Interface_Object;
typedef struct Interface_TabCloseButton Interface_TabCloseButton;
typedef struct Interface_Tab            Interface_Tab;
typedef struct Interface_NewTabButton   Interface_NewTabButton;
typedef struct Interface_TabBar         Interface_TabBar;
typedef struct Interface_EditView       Interface_EditView;
typedef struct Interface_EditViewRuler  Interface_EditViewRuler;
typedef struct Interface_EditViewText   Interface_EditViewText;
typedef struct Interface                Interface;

Error Interface_run           (void);
void  Interface_setEditBuffer (EditBuffer *newEditBuffer);

Interface_Tab *Interface_tabBar_add       (size_t, const char *);
void           Interface_tabBar_delete    (Interface_Tab *);
void           Interface_tabBar_setActive (Interface_Tab *);
void           Interface_Tab_setText      (Interface_Tab *, const char *);
size_t         Interface_Tab_getBufferId  (Interface_Tab *);
Interface_Tab *Interface_Tab_getPrevious  (Interface_Tab *);
Interface_Tab *Interface_Tab_getNext      (Interface_Tab *);
int            Interface_Tab_isActive     (Interface_Tab *);

void Interface_onStart     (void (*) (void));
void Interface_onNewTab    (void (*) (void));
void Interface_onCloseTab  (void (*) (Interface_Tab *));
void Interface_onSwitchTab (void (*) (Interface_Tab *));

// make event handlers for creating a new tab, clicking on a tab, etc
