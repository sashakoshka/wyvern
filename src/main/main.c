#include <unistd.h>
#include <stdio.h>
#include "options.h"
#include "interface.h"
#include "edit-buffer.h"
#include "buffer-manager.h"

static void   handleStart     (void);
static void   handleSwitchTab (Interface_Tab *);
static size_t newTab          (void);

int main () {
        BufferManager_init();
        Options_start();
        Interface_onStart(handleStart);
        Interface_onSwitchTab(handleSwitchTab);
        Interface_run();
}

static void handleStart (void) {
        newTab();
        newTab();
        newTab();
}

static void handleSwitchTab (Interface_Tab *tab) {
        Interface_tabBar_setActive(tab);
        size_t bufferId = Interface_Tab_getBufferId(tab);
        Interface_setEditBuffer(BufferManager_get(bufferId));
}

static size_t newTab (void) {
        EditBuffer *editBuffer = EditBuffer_new();
        EditBuffer_open(editBuffer, "src/edit-buffer/edit-buffer.c");
        
        size_t bufferId = BufferManager_add(editBuffer);
        Interface_Tab *tab = Interface_tabBar_add(bufferId);

        Interface_Tab_setText(tab, "Untitled");
        Interface_tabBar_setActive(tab);
        Interface_setEditBuffer(BufferManager_get(bufferId));

        return bufferId;
}
