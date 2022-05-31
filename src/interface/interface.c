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
        interface.editView.text.buffer = newEditBuffer;
        TextDisplay_setModel (
                interface.editView.text.display,
                interface.editView.text.buffer);
}

/* Interface_setup
 * Initializes the interface, loading needed files and setting event handlers on
 * the windows.
 */
static Error Interface_setup (void) {
        Error err = Interface_loadFonts();
        if (err) { return err; }
        
        if (interface.editView.text.display != NULL) {
                free(interface.editView.text.display);
        }
        interface.editView.text.display = TextDisplay_new (
                interface.editView.text.buffer,
                (size_t)interface.width,
                (size_t)interface.height);
                
        Window_interval = 500;
        Window_setTitle("Text Editor");
        
        Window_onRedraw      (Interface_handleRedraw);
        Window_onMouseButton (Interface_handleMouseButton);
        Window_onMouseMove   (Interface_handleMouseMove);
        Window_onInterval    (Interface_handleInterval);
        Window_onKey         (Interface_handleKey);

        interface.tabBar.newTabButton.redrawOnHover       = 1;
        interface.tabBar.newTabButton.redrawOnMouseButton = 1;

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

/* Interface_Object_invalidateLayoutBack
 * Invalidates the layout of a generic object. This should not be called on
 * objects with children, as those children will not be processed.
 */
void Interface_Object_invalidateLayoutBack (Interface_Object *object) {
        object->needsRedraw = 1;
        object->needsRecalculate = 1;
}

/* Interface_Object_invalidateDrawingBack
 * Invalidates the drawing of a generic object. This should not be called on
 * objects with children, as those children will not be processed.
 */
void Interface_Object_invalidateDrawingBack (Interface_Object *object) {
        object->needsRedraw = 1;
}

/* Interface_getHoveredObject
 * Returns the object that the x and y coordinates are within.
 */
Interface_Object *Interface_getHoveredObject (int x, int y) {
        Interface_Object *object;

        object = Interface_tabBar_getHoveredObject(x, y);
        if (object != NULL) { return object; }

        object = Interface_editView_getHoveredObject(x, y);
        if (object != NULL) { return object; }

        return TO_GENERIC(&interface);
}

/* Interface_Object_isWithinBoundsBack
 * Returns 1 if the x and y coordinates are within the object's bounds.
 */
int Interface_Object_isWithinBoundsBack (
        Interface_Object *object,
        int x,
        int y
) {
        return 
                x > (object->x) && x < (object->x) + (object->width) &&
                y > (object->y) && y < (object->y) + (object->height);
}
