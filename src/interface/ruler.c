#include "module.h"
#include "options.h"

/* Interface_editViewRuler_recalculate
 * Recalculates the size and position of the ruler.
 */
void Interface_editViewRuler_recalculate (void) {
        Interface_EditView      *editView = &interface.editView;
        Interface_EditViewRuler *ruler    = &editView->ruler;

        ruler->x = editView->x;
        ruler->y = editView->y;
        ruler->width  = (int)(interface.fonts.glyphWidth * 5);
        ruler->height = editView->height;
}

/* Interface_editViewRuler_redraw
 * Redraws the line number ruler.
 */
void Interface_editViewRuler_redraw (void) {
        Interface_EditView      *editView = &interface.editView;
        Interface_EditViewText  *text     = &editView->text;
        Interface_EditViewRuler *ruler    = &editView->ruler;
        
        cairo_set_source_rgb(Window_context, RULER_COLOR);
        cairo_rectangle (
                Window_context,
                editView->x,
                editView->y,
                editView->innerX + ruler->width,
                editView->height);
        cairo_fill(Window_context);
        
        double y = text->y;
        for (
                size_t index = text->buffer->scroll;
                index < text->buffer->length &&
                y < text->y + text->height;
                index ++
        ) {     
                char lineNumberBuffer[8] = { 0 };
                snprintf(lineNumberBuffer, 7, "%zu", index + 1);
                
                cairo_set_source_rgb(Window_context, RULER_TEXT_COLOR);
                cairo_move_to (
                        Window_context,
                        editView->innerX, y);
                cairo_show_text(Window_context, lineNumberBuffer);

                y += interface.fonts.lineHeight;
        }
}

/* Interface_editViewRuler_invalidateLayout
 * Invalidates the layout of the ruler.
 */
void Interface_editViewRuler_invalidateLayout (void) {
        Interface_EditViewRuler *ruler = &interface.editView.ruler;
        ruler->needsRedraw = 1;
        ruler->needsRecalculate = 1;
}

/* Interface_editViewRuler_invalidateDrawing
 * Invalidates the drawing of the ruler.
 */
void Interface_editViewRuler_invalidateDrawing (void) {
        Interface_EditViewRuler *ruler = &interface.editView.ruler;
        ruler->needsRedraw = 1;
}
