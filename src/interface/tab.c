#include "module.h"
#include "utility.h"

/* Interface_Tab_redraw
 * Recalculates a single tab.
 */
void Interface_Tab_recalculate (Interface_Tab *tab) {
        // tab position
        tab->x = 0 + interface.tabBar.x;
        tab->y = 0 + interface.tabBar.y;

        if (tab->previous == NULL) {
                tab->x -= interface.tabBar.scroll;
        } else {
                tab->x += tab->previous->x + tab->previous->width;
        }

        // tab height
        tab->height = interface.tabBar.height - 1;

        // text
        cairo_text_extents_t textExtents;
        cairo_text_extents(Window_context, tab->text, &textExtents);
        double padding =
                (tab->height - interface.fonts.capitalHeight) / 2;
        tab->textX = tab->x + padding;
        tab->textY = tab->y + tab->height - padding;

        // close button dimensions
        tab->closeButton.width  = 20;
        tab->closeButton.height = tab->closeButton.width;
        double closeMargin = (tab->height - tab->closeButton.height) / 2;

        // tab width
        tab->width =
                padding +
                textExtents.width +
                closeMargin +
                tab->closeButton.width +
                closeMargin;

        // close button position
        tab->closeButton.x =
                tab->x + tab->width -
                closeMargin -
                tab->closeButton.width;
        tab->closeButton.y = tab->y + closeMargin;
}

/* Interface_Tab_redraw
 * Redraws a single tab.
 */
void Interface_Tab_redraw (Interface_Tab *tab) {
        cairo_push_group(Window_context);

        // tab background
        if (tab == interface.tabBar.activeTab) {
                cairo_set_source_rgb(Window_context, ACTIVE_TAB_COLOR);
        } else if (
                Interface_Object_isHovered(tab) ||
                Interface_Object_isHovered(&tab->closeButton)
        ) {
                cairo_set_source_rgb(Window_context, HOVER_TAB_COLOR);
        } else {
                cairo_set_source_rgb(Window_context, INACTIVE_TAB_COLOR);
        }
        cairo_rectangle (
                Window_context,
                tab->x,
                tab->y,
                tab->width,
                tab->height);
        cairo_fill(Window_context);

        // right border
        cairo_set_source_rgb(Window_context, OUTLINE_COLOR);
        cairo_set_line_width(Window_context, 1);
        cairo_move_to (
                Window_context,
                tab->x + tab->width - 0.5,
                tab->y + 0.5);
        cairo_line_to (
                Window_context,
                tab->x + tab->width  - 0.5,
                tab->y + tab->height + 0.5);
        cairo_stroke(Window_context);

        if (tab == interface.tabBar.activeTab) {
                cairo_set_source_rgb(Window_context, ACCENT_COLOR);
                cairo_set_line_width(Window_context, 2);
                cairo_move_to (
                        Window_context,
                        tab->x,
                        tab->y + tab->height - 1);
                cairo_line_to (
                        Window_context,
                        tab->x + tab->width - 1,
                        tab->y + tab->height - 1);
                cairo_stroke(Window_context);
        }

        // text
        if (tab == interface.tabBar.activeTab) {
                cairo_set_source_rgb(Window_context, ACTIVE_TAB_TEXT_COLOR);
        } else {
                cairo_set_source_rgb(Window_context, INACTIVE_TAB_TEXT_COLOR);
        }
        Interface_fontNormal();
        cairo_move_to (
                Window_context,
                tab->textX, tab->textY);
        cairo_show_text(Window_context, tab->text);

        // clip so that the new tab button will not overlap
        cairo_pop_group_to_source(Window_context);
        cairo_rectangle (
                Window_context,
                interface.tabBar.x,
                interface.tabBar.y,
                interface.tabBar.tabClippingPoint,
                interface.tabBar.height);
        cairo_fill(Window_context);
}

/* Interface_Tab_setText
 * Sets the text that will be displayed as the tab title.
 */
void Interface_Tab_setText (Interface_Tab *tab, const char *text) {
        Utility_copyCString(tab->text, text, NAME_MAX);
}

/* Interface_Tab_getBufferId
 * Returns the buffer id of the tab. This can be used as a key for BufferManger.
 */
size_t Interface_Tab_getBufferId (Interface_Tab *tab) {
        return tab->bufferId;
}

/* Interface_Tab_getPrevious
 * Returns the tab right behind this tab.
 */
Interface_Tab *Interface_Tab_getPrevious (Interface_Tab *tab) {
        return tab->previous;
}

/* Interface_Tab_getNext
 * Returns the tab right in front of this tab.
 */
Interface_Tab *Interface_Tab_getNext (Interface_Tab *tab) {
        return tab->next;
}

/* Interface_Tab_isActive
 * Rethers whether or not the tab is currently active.
 */
int Interface_Tab_isActive (Interface_Tab *tab) {
        return interface.tabBar.activeTab == tab;
}

/* Interface_Tab_new
 * Allocates and returns new tab.
 */
Interface_Tab *Interface_Tab_new (void) {
        Interface_Tab *tab = calloc(1, sizeof(*tab));
        tab->redrawOnHover = 1;
        tab->closeButton.tab = tab;
        tab->closeButton.redrawOnHover = 1;
        tab->closeButton.redrawOnMouseButton = 1;
        return tab;
}

/* Interface_Tab_free
 * Frees a tab.
 */
void Interface_Tab_free (Interface_Tab *tab) {
        Interface_Object_detatchReferences(tab);
        Interface_Object_detatchReferences(&tab->closeButton);
        free(tab);
}

/* Interface_Tab_invalidateLayout
 * Invalidates the layout of a single tab.
 */
void Interface_Tab_invalidateLayout (Interface_Tab *tab) {
        Interface_Object_invalidateLayout(tab);
        Interface_Object_invalidateLayout(&tab->closeButton);
}

/* Interface_Tab_invalidateDrawing
 * Invalidates the drawing of a single tab.
 */
void Interface_Tab_invalidateDrawing (Interface_Tab *tab) {
        Interface_Object_invalidateDrawing(tab);
        Interface_Object_invalidateDrawing(&tab->closeButton);
}

/* Interface_Tab_refresh
 * Recalculates the tab if it needs to be recalculated, and redraws if it
 * needs to redrawn.
 */
void Interface_Tab_refresh (Interface_Tab *tab) {
        if (tab->needsRecalculate) {
                Interface_Tab_recalculate(tab);
                tab->needsRecalculate = 0;
        }
        
        if (tab->needsRedraw) {
                Interface_Tab_redraw(tab);
                tab->needsRedraw = 0;
        }

        if (tab->closeButton.needsRedraw) {
                Interface_TabCloseButton_redraw(&tab->closeButton);
        }
}

/* Interface_Tab_getHoveredObject
 * Returns the object that the x and y coordinates are within.
 */
Interface_Object *Interface_Tab_getHoveredObject (
        Interface_Tab *tab,
        int x, int y
) {
        if (!Interface_Object_isWithinBounds(tab, x, y)) { return NULL; }
        
        if (Interface_Object_isWithinBounds(&tab->closeButton, x, y)) {
                return TO_GENERIC(&tab->closeButton);
        }

        return TO_GENERIC(tab);
}


/* Interface_TabCloseButton_refresh
 * Recalculates a tab close button if it needs to be recalculated, and redraws
 * if it needs to redrawn.
 */
void Interface_TabCloseButton_refresh (Interface_TabCloseButton *closeButton) {
        if (closeButton->needsRedraw) {
                Interface_TabCloseButton_redraw(closeButton);
                closeButton->needsRedraw = 0;
        }
}

/* Interface_TabCloseButton_redraw
 * Redraws the tab's close button.
 */
void Interface_TabCloseButton_redraw (Interface_TabCloseButton *closeButton) {
        cairo_push_group(Window_context);
        
        if (closeButton->tab == interface.tabBar.activeTab) {
                cairo_set_source_rgb(Window_context, ACTIVE_TAB_COLOR);
        } else if (
                Interface_Object_isHovered(closeButton->tab) ||
                Interface_Object_isHovered(closeButton)
        ) {
                cairo_set_source_rgb(Window_context, HOVER_TAB_COLOR);
        } else {
                cairo_set_source_rgb(Window_context, INACTIVE_TAB_COLOR);
        }
        cairo_rectangle (
                Window_context,
                closeButton->x,
                closeButton->y,
                closeButton->width,
                closeButton->height);
        cairo_fill(Window_context);

        if (Interface_Object_isClicked(closeButton)) {
                cairo_set_source_rgb(Window_context, CLOSE_BUTTON_CLICK_COLOR);
        } else if (Interface_Object_isHovered(closeButton)) {
                cairo_set_source_rgb(Window_context, CLOSE_BUTTON_HOVER_COLOR);
        } else {
                cairo_set_source_rgb(Window_context, BUTTON_SYMBOL_COLOR);
        }
        double nearX = closeButton->x + 7;
        double nearY = closeButton->y + 7;
        double farX  = closeButton->x + closeButton->width  - 7;
        double farY  = closeButton->y + closeButton->height - 7;
        cairo_set_line_width(Window_context, 1);
        cairo_move_to (
                Window_context,
                nearX,
                nearY);
        cairo_line_to (
                Window_context,
                farX,
                farY);
                cairo_stroke(Window_context);
        cairo_move_to (
                Window_context,
                farX,
                nearY);
        cairo_line_to (
                Window_context,
                nearX,
                farY);
                cairo_stroke(Window_context);

        // clip so that the new tab button will not overlap
        cairo_pop_group_to_source(Window_context);
        cairo_rectangle (
                Window_context,
                interface.tabBar.x,
                interface.tabBar.y,
                interface.tabBar.tabClippingPoint,
                interface.tabBar.height);
        cairo_fill(Window_context);
}
