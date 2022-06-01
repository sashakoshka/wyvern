#include <unistd.h>
#include <stdio.h>
#include "options.h"
#include "interface.h"
#include "edit-buffer.h"

static void handleStart     (void);
static void handleSwitchTab (Interface_Tab *);

int main () {
        Options_start();
        Interface_onStart(handleStart);
        Interface_onSwitchTab(handleSwitchTab);
        Interface_run();
}

static void handleStart (void) {
        EditBuffer *editBuffer = EditBuffer_new();
        
        EditBuffer_open(editBuffer, "src/edit-buffer/edit-buffer.c");
        Interface_setEditBuffer(editBuffer);
        
        Interface_Tab *tab = Interface_tabBar_add();
        Interface_Tab_setText(tab, "rather long title");
        
        tab = Interface_tabBar_add();
        Interface_Tab_setText(tab, "another tab");
        Interface_tabBar_setActive(tab);
}

static void handleSwitchTab (Interface_Tab *tab) {
        Interface_tabBar_setActive(tab);
}
