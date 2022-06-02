#include <unistd.h>
#include <stdio.h>
#include "options.h"
#include "interface.h"
#include "edit-buffer.h"
#include "buffer-manager.h"

static void   handleStart     (void);
static void   handleSwitchTab (Interface_Tab *);
static void   handleNewTab    (void);
static size_t openInNewTab    (const char *);

int main () {
        BufferManager_init();
        Options_start();
        Interface_onStart(handleStart);
        Interface_onSwitchTab(handleSwitchTab);
        Interface_onNewTab(handleNewTab);
        Interface_run();
}

static void handleStart (void) {
        openInNewTab(NULL);
        openInNewTab("src/interface/interface.c");
        openInNewTab("src/edit-buffer/edit-buffer.c");
}

static void handleSwitchTab (Interface_Tab *tab) {
        Interface_tabBar_setActive(tab);
        size_t bufferId = Interface_Tab_getBufferId(tab);
        Interface_setEditBuffer(BufferManager_get(bufferId));
}

static void handleNewTab (void) {
        openInNewTab(NULL);
}

static size_t openInNewTab (const char *path) {
        EditBuffer *editBuffer = EditBuffer_new();
        EditBuffer_open(editBuffer, path);
        
        size_t bufferId = BufferManager_add(editBuffer);
        Interface_Tab *tab = Interface_tabBar_add(bufferId, "Untitled");

        Interface_tabBar_setActive(tab);
        Interface_setEditBuffer(BufferManager_get(bufferId));

        return bufferId;
}
