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
        interface.width  = width;
        interface.height = height;

        int horizontal = interface.width > interface.height;

        interface.tabBar.x      = 0;
        interface.tabBar.y      = 0;
        interface.tabBar.height = 35;
        interface.tabBar.width  = interface.width;

        Interface_EditView *editView = &interface.editView;
        
        editView->x      = 0;
        editView->y      = interface.tabBar.height;
        editView->height = interface.height - interface.tabBar.height;
        editView->width  = interface.width;
        
        editView->padding    = (int)glyphWidth * 2;
        editView->rulerWidth = (int)(glyphWidth * 5);

        editView->innerX      = editView->x      + editView->padding;
        editView->innerY      = editView->y      + editView->padding;
        editView->innerWidth  = editView->width  - editView->padding;
        editView->innerHeight = editView->height - editView->padding;

        double textLeftOffset = editView->rulerWidth + editView->padding;
        editView->textX = editView->innerX + textLeftOffset;
        editView->textY = editView->innerY + glyphHeight * 0.8;
        editView->textWidth  = editView->width  - textLeftOffset;
        editView->textHeight = editView->height - editView->padding +
                lineHeight;

        double textDisplayWidth  = editView->textWidth  / glyphWidth;
        double textDisplayHeight = editView->textHeight / lineHeight;
        if (textDisplayWidth  < 0) { textDisplayWidth  = 0; }
        if (textDisplayHeight < 0) { textDisplayHeight = 0; }
        TextDisplay_resize (
                textDisplay,
                (size_t)(textDisplayWidth),
                (size_t)(textDisplayHeight));
        
        if (horizontal) {
                
        }
}

void Interface_redraw (void) {
        Interface_tabBar_redraw();
        Interface_editView_redraw();
}
