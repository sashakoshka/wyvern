#include "module.h"

/* Interface_tabBar_recalculate
 * Recalculates the position and size of the tab bar.
 */
void Interface_tabBar_recalculate (void) {
        interface.tabBar.x      = 0;
        interface.tabBar.y      = 0;
        interface.tabBar.height = 35;
        interface.tabBar.width  = interface.width;
}

/* Interface_tabBar_redraw
 * Re-draws the tab bar.
 */
void Interface_tabBar_redraw (void) {
        cairo_set_source_rgb(Window_context, TAB_BAR_COLOR);
        cairo_rectangle (
                Window_context,
                interface.tabBar.x,
                interface.tabBar.y,
                interface.tabBar.width,
                interface.tabBar.height);
        cairo_fill(Window_context);
                
        cairo_set_source_rgb(Window_context, OUTLINE_COLOR);
        cairo_set_line_width(Window_context, 1);
        cairo_move_to (
                Window_context,
                interface.tabBar.x,
                interface.tabBar.y + interface.tabBar.height - 0.5);
        cairo_line_to (
                Window_context,
                interface.tabBar.x + interface.tabBar.width,
                interface.tabBar.y + interface.tabBar.height - 0.5);
        cairo_stroke(Window_context);
}
