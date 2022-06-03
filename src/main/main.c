#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include "options.h"
#include "interface.h"
#include "edit-buffer.h"
#include "buffer-manager.h"

static void   handleStart     (void);
static void   handleSwitchTab (Interface_Tab *);
static void   handleNewTab    (void);
static void   handleCloseTab  (Interface_Tab *);
static size_t openInNewTab    (char *);

int main () {
        BufferManager_init();
        Options_start();
        Interface_onStart(handleStart);
        Interface_onSwitchTab(handleSwitchTab);
        Interface_onNewTab(handleNewTab);
        Interface_onCloseTab(handleCloseTab);
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

static void handleCloseTab (Interface_Tab *tab) {
        BufferManager_delete(Interface_Tab_getBufferId(tab));
        
        if (Interface_Tab_isActive(tab)) {
                Interface_Tab *switchTo = Interface_Tab_getNext(tab);

                if (switchTo == NULL) {
                        switchTo = Interface_Tab_getPrevious(tab);
                }
                
                Interface_tabBar_setActive(switchTo);

                EditBuffer *bufferSwitchTo = NULL;
                if (switchTo != NULL) {
                        size_t previousBufferId =
                                Interface_Tab_getBufferId(switchTo);
                        bufferSwitchTo = BufferManager_get(previousBufferId);
                }
                
                Interface_setEditBuffer(bufferSwitchTo);
        }
        
        Interface_tabBar_delete(tab);
}

static size_t openInNewTab (char *path) {
        EditBuffer *editBuffer = EditBuffer_new();
        EditBuffer_open(editBuffer, path);
        
        size_t bufferId = BufferManager_add(editBuffer);

        char *tabTitle = "Untitled";
        if (path != NULL) {
                tabTitle = basename(path);
        }
        Interface_Tab *tab = Interface_tabBar_add(bufferId, tabTitle);

        Interface_tabBar_setActive(tab);
        Interface_setEditBuffer(BufferManager_get(bufferId));

        return bufferId;
}
