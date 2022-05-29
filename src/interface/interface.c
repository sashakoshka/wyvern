#include "module.h"
#include "utility.h"

Interface interface = { 0 };

static Error Interface_setup (void);

/* Interface_run
 * Initializes and runs the interface module. This function is blocking, and
 * will return when the window is closed.
 */
Error Interface_run (void) {
        Error err;
        
        err = Window_start();
        if (err) { return err; }
        
        err = Interface_setup();
        if (err) { return err; }

        interface.callbacks.onStart();
        
        Window_show();
        
        err = Window_listen();
        Window_stop();

        return err;
}

/* Interface_setEditBuffer
 * Sets the active EditBuffer of the interface.
 */
void Interface_setEditBuffer (EditBuffer *newEditBuffer) {
        interface.editView.editBuffer = newEditBuffer;
        TextDisplay_setModel (
                interface.editView.textDisplay,
                interface.editView.editBuffer);
}

/* Interface_setup
 * Initializes the interface, loading needed files and setting event handlers on
 * the windows.
 */
static Error Interface_setup (void) {
        Error err = Interface_loadFonts();
        if (err) { return err; }
        
        if (interface.editView.textDisplay != NULL) {
                free(interface.editView.textDisplay);
        }
        interface.editView.textDisplay = TextDisplay_new (
                interface.editView.editBuffer,
                (size_t)interface.width,
                (size_t)interface.height);
        
        Window_onRedraw      (Interface_handleRedraw);
        Window_onMouseButton (Interface_handleMouseButton);
        Window_onMouseMove   (Interface_handleMouseMove);
        Window_onInterval    (Interface_handleInterval);
        Window_onKey         (Interface_handleKey);
        
        Window_interval = 500;
        Window_setTitle("Text Editor");

        return Error_none;
}

/* Interface_recalculate
 * Recalculates the size and position of the base interface.
 */
void Interface_recalculate () {
        interface.x = 0;
        interface.y = 0;
        interface.horizontal = interface.width > interface.height;
}

/* Interface_redraw
 * Re-draws the base interface.
 */
void Interface_redraw (void) {
        
}

/* Interface_refresh
 * Recalculates elements that need to be recalculated, and redraws elements that
 * need to be redrawn.
 */
void Interface_refresh () {
        if (interface.needsRecalculate == 1) {
                Interface_recalculate();
                interface.needsRecalculate = 0;
        }

        if (interface.needsRedraw == 1) {
                Interface_redraw();
                interface.needsRedraw = 0;
        }

        Interface_tabBar_refresh();
        Interface_editView_refresh();
}

/* Interface_invalidateLayout
 * Recursively invalidates the layout of the entire interface.
 */
void Interface_invalidateLayout (void) {
        interface.needsRedraw = 1;
        interface.needsRecalculate = 1;
        Interface_tabBar_invalidateLayout();
        Interface_editView_invalidateLayout();
}

/* Interface_invalidateDrawing
 * Recursively invalidates the drawing of the entire interface.
 */
void Interface_invalidateDrawing (void) {
        interface.needsRedraw = 1;
        Interface_tabBar_invalidateDrawing();
        Interface_editView_invalidateDrawing();
}
