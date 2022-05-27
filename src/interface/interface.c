#include "module.h"

Interface    interface   = { 0 };
EditBuffer  *editBuffer  = { 0 };
TextDisplay *textDisplay = { 0 };

static Error Interface_setup (void);

Error Interface_run (void) {
        Window_start();
        Error err = Interface_setup();
        if (err) { return err; }
        Window_show();
        
        err = Window_listen();
        Window_stop();

        return err;
}

void Interface_setEditBuffer (EditBuffer *newEditBuffer) {
        editBuffer = newEditBuffer;
}

static Error Interface_setup (void) {
        Error err = Interface_loadFonts();
        if (err) { return err; }
        
        if (textDisplay != NULL) { free(textDisplay); }
        textDisplay = TextDisplay_new (
                editBuffer,
                (size_t)interface.width,
                (size_t)interface.height);
        
        Window_onRedraw      (Interface_onRedraw);
        Window_onMouseButton (Interface_onMouseButton);
        Window_onMouseMove   (Interface_onMouseMove);
        Window_onInterval    (Interface_onInterval);
        Window_onKey         (Interface_onKey);
        
        Window_interval = 500;
        Window_setTitle("Text Editor");

        return Error_none;
}

void Interface_recalculate (int width, int height) {
        interface.width      = width;
        interface.height     = height;
        interface.horizontal = interface.width > interface.height;
        
        Interface_tabBar_recalculate();
        Interface_editView_recalculate();
}

void Interface_redraw (void) {
        Interface_tabBar_redraw();
        Interface_editView_redraw();
}
