#include "module.h"

void Interface_Tab_redraw (Interface_Tab *tab) {
        cairo_set_source_rgb(Window_context, INACTIVE_TAB_COLOR);
        cairo_rectangle (
                Window_context,
                tab->x,
                tab->y,
                tab->width,
                tab->height);
        cairo_fill(Window_context);
}
