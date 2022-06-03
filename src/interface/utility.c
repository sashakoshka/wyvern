#include <math.h>
#include "module.h"

/* Interface_roundedRectangle
 * Draws a rounded rectangle.
 */
void Interface_roundedRectangle (
        double x,
        double y,
        double width,
        double height,
        double radius
) {
        double degrees = M_PI / 180.0;

        cairo_new_sub_path (Window_context);
        cairo_arc (
                Window_context,
                x + width - radius,
                y + radius, radius,
                -90 * degrees, 0 * degrees);
        cairo_arc (
                Window_context,
                x + width - radius,
                y + height - radius, radius,
                0 * degrees, 90 * degrees);
        cairo_arc (
                Window_context,
                x + radius,
                y + height - radius, radius,
                90 * degrees, 180 * degrees);
        cairo_arc (
                Window_context,
                x + radius,
                y + radius, radius,
                180 * degrees, 270 * degrees);
        cairo_close_path (Window_context);
}
