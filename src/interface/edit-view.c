#include "module.h"
#include "options.h"

/* Interface_editView_recalculate
 * Recalculates the position, size, and various layout bounds of the edit view.
 */
void Interface_editView_recalculate (void) {
        Interface_EditView      *editView = &interface.editView;
        
        editView->x      = 0;
        editView->y      = interface.tabBar.height;
        editView->height = interface.height - interface.tabBar.height;
        editView->width  = interface.width;
        
        editView->padding = (int)(interface.fonts.glyphWidth * 2);
        
        editView->innerX      = editView->x      + editView->padding;
        editView->innerY      = editView->y      + editView->padding;
        editView->innerWidth  = editView->width  - editView->padding;
        editView->innerHeight = editView->height - editView->padding;

        Interface_editViewRuler_recalculate();
        Interface_editViewText_recalculate();
}

/* Interface_editView_redraw
 * Completely redraws the edit view.
 */
void Interface_editView_redraw (void) {
        Interface_EditView     *editView = &interface.editView;
        Interface_EditViewText *text     = &editView->text;
        
        cairo_set_source_rgb(Window_context, BACKGROUND_COLOR);
        cairo_rectangle (
                Window_context,
                editView->x,
                editView->y,
                editView->width,
                editView->height);
        cairo_fill(Window_context);

        cairo_set_source_rgb(Window_context, RULER_COLOR);
        cairo_set_line_width(Window_context, 2);
        double columnMarkerX =
                text->x + interface.fonts.glyphWidth * 80 + 1;
        cairo_move_to(Window_context, columnMarkerX, editView->y);
        cairo_line_to (
                Window_context,
                columnMarkerX,
                editView->y + editView->height);
        cairo_stroke(Window_context);
}

/* Interface_findMouseHoverCell
 * Takes in mouse coordinates and finds out the coordinates of the cell they are
 * hovering over. If the given coordinates exceed the bounds of the text view,
 * they are constrained to it.
 */
void Interface_findMouseHoverCell (
        int mouseX,
        int mouseY,
        size_t *cellX,
        size_t *cellY
) {
        Interface_EditView     *editView = &interface.editView;
        Interface_EditViewText *text     = &editView->text;
        
        int intCellX = (int) (
                (mouseX - text->x) /
                interface.fonts.glyphWidth);
        int intCellY = (int) (
                (mouseY - editView->innerY) /
                interface.fonts.lineHeight);

        *cellX = (size_t)(intCellX);
        *cellY = (size_t)(intCellY);
        
        if (intCellX < 0) { *cellX = 0; }
        if (intCellY < 0) { *cellY = 0; }
        
        if (*cellX >= text->display->width) {
                *cellX = text->display->width - 1;
        }
        if (*cellY >= text->display->height) {
                *cellY = text->display->height - 1;
        }
}

/* Interface_updateTextSelection
 * Updates the text selection according to the mouse position. This should only
 * be called when the user is actively selecting text with the mouse.
 */
void Interface_updateTextSelection (void) {
        Interface_EditView     *editView = &interface.editView;
        Interface_EditViewText *text     = &editView->text;
        
        size_t cellX = 0;
        size_t cellY = 0;
        Interface_findMouseHoverCell (
                interface.mouseState.x, interface.mouseState.y,
                &cellX, &cellY);
        
        size_t realX;
        size_t realY;
        TextDisplay_getRealCoords (
                text->display,
                cellX, cellY,
                &realX, &realY);

        EditBuffer_Cursor_moveTo (
                text->buffer->cursors,
                interface.mouseState.dragOriginRealX,
                interface.mouseState.dragOriginRealY);
        EditBuffer_Cursor_selectTo (
                text->buffer->cursors,
                realX, realY);
}

/* Interface_editView_refresh
 * Refreshes the edit view.
 */
void Interface_editView_refresh (void) {
        if (interface.editView.needsRecalculate == 1) {
                Interface_editView_recalculate();
                interface.editView.needsRecalculate = 0;
        }

        if (interface.editView.needsRedraw == 1) {
                Interface_editView_redraw();
                interface.editView.needsRedraw = 0;
        }

        // Interface_editViewRuler_refresh();
        // Interface_editViewText_refresh();
}

/* Interface_editView_invalidateLayout
 * Invalidates the layout of the edit view.
 */
void Interface_editView_invalidateLayout (void) {
        Interface_EditView *editView = &interface.editView;
        
        editView->needsRedraw = 1;
        editView->needsRecalculate = 1;
        Interface_editViewRuler_invalidateLayout();
        Interface_editViewText_invalidateLayout();
}

/* Interface_editView_invalidateDrawing
 * Invalidates the drawing of the edit view.
 */
void Interface_editView_invalidateDrawing (void) {
        Interface_EditView *editView = &interface.editView;
        
        editView->needsRedraw = 1;
        Interface_editViewRuler_invalidateDrawing();
        Interface_editViewText_invalidateDrawing();
}
