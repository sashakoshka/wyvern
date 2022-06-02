#include "module.h"
#include "utility.h"

/* Interface_tabBar_recalculate
 * Recalculates the position and size of the tab bar.
 */
void Interface_tabBar_recalculate (void) {
        interface.tabBar.x      = 0;
        interface.tabBar.y      = 0;
        interface.tabBar.height = 35;
        interface.tabBar.width  = interface.width;

        interface.tabBar.tabClippingPoint =
                interface.tabBar.width - interface.tabBar.height;

        interface.tabBar.layoutIsValid = 1;
}

/* Interface_tabBar_redraw
 * Redraws the tab bar.
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

/* Interface_tabBar_add
 * Appends a new tab to the linked list in the tab bar.
 */
Interface_Tab *Interface_tabBar_add (size_t bufferId, const char *text) {
        Interface_Tab *tab = Interface_Tab_new();
        tab->bufferId = bufferId;
        Interface_Tab_setText(tab, text);

        if (interface.tabBar.tabs == NULL) {
                interface.tabBar.tabs = tab;
                return tab;
        }

        Interface_Tab *current = interface.tabBar.tabs;
        while (current->next != NULL) {
                current = current->next;
        }

        current->next = tab;
        tab->previous = current;

        // if we can, we need to recalculate this tab, so that
        // Interface_tabBar_setActive will be able to scroll to it.
        if (interface.tabBar.layoutIsValid) {
                Interface_Tab_recalculate(tab);
        }
        Interface_tabBar_invalidateLayout();
        
        return tab;
}

/* Interface_tabBar_delete
 * Removes an existing tab from the linked list in the tab bar.
 */
void Interface_tabBar_delete (Interface_Tab *tab) {
        if (tab->previous == NULL) {
                interface.tabBar.tabs = tab->next;
                Interface_Tab_free(tab);
                return;
        }

        tab->previous->next = tab->next;
        if (tab->next != NULL) {
                tab->next->previous = tab->previous;
        }
        
        Interface_Tab_free(tab);
}

/* Interface_tabBar_setActive
 * Sets the currently active tab. This function does not trigger any events, and
 * should be called from an event handler callback.
 */
void Interface_tabBar_setActive (Interface_Tab *tab) {
        Interface_TabBar *tabBar = &interface.tabBar;
        tabBar->activeTab = tab;

        if (tab->x < tabBar->x) {
               tabBar->scroll += (int)(tab->x - tabBar->x);
        } else if (
                tab->x + tab->width >
                tabBar->x + tabBar->tabClippingPoint
        ) {
                tabBar->scroll -= (int)(
                        (tabBar->x + tabBar->tabClippingPoint) -
                        (tab->x + tab->width));
        }
        Interface_invalidateLayout();
}

/* Interface_tabBar_invalidateLayout
 * Invalidates the layout of the tab bar.
 */
void Interface_tabBar_invalidateLayout (void) {
        if (interface.tabBar.needsRecalculate == 1) { return; }
        interface.tabBar.needsRedraw = 1;
        interface.tabBar.needsRecalculate = 1;

        Interface_Tab *tab = interface.tabBar.tabs;
        while (tab != NULL) {
                Interface_Tab_invalidateLayout(tab);
                tab = tab->next;
        }

        Interface_Object_invalidateLayout(&interface.tabBar.newTabButton);
}

/* Interface_tabBar_invalidateLayout
 * Invalidates the drawing of the tab bar.
 */
void Interface_tabBar_invalidateDrawing (void) {
        if (interface.tabBar.needsRedraw == 1) { return; }
        interface.tabBar.needsRedraw = 1;

        Interface_Tab *tab = interface.tabBar.tabs;
        while (tab != NULL) {
                Interface_Tab_invalidateDrawing(tab);
                tab = tab->next;
        }
        
        Interface_Object_invalidateDrawing(&interface.tabBar.newTabButton);
}

/* Interface_tabBar_refresh
 * Recalculates the tab bar if it needs to be recalculated, and redraws if it
 * needs to redrawn.
 */
void Interface_tabBar_refresh (void) {
        if (interface.tabBar.needsRecalculate == 1) {
                Interface_tabBar_recalculate();
                interface.tabBar.needsRecalculate = 0;
        }
        
        if (interface.tabBar.needsRedraw == 1) {
                Interface_tabBar_redraw();
                interface.tabBar.needsRedraw = 0;
        }

        Interface_Tab *tab = interface.tabBar.tabs;
        while (tab != NULL) {
                Interface_Tab_refresh(tab);
                tab = tab->next;
        }

        Interface_newTabButton_refresh();
}

/* Interface_newTabButton_recalculate
 * Recalculates the size amd position of the new tab button.
 */
void Interface_newTabButton_recalculate (void) {
        Interface_TabBar       *tabBar       = &interface.tabBar;
        Interface_NewTabButton *newTabButton = &tabBar->newTabButton;

        newTabButton->padding = 4;

        newTabButton->height = tabBar->height;
        newTabButton->width  = newTabButton->height;

        newTabButton->innerHeight =
                newTabButton->height -
                newTabButton->padding * 2;
        newTabButton->innerWidth =
                newTabButton->width -
                newTabButton->padding * 2;
                
        newTabButton->x =
                tabBar->x +
                tabBar->width -
                newTabButton->width;
        newTabButton->y = tabBar->y;
        
        newTabButton->innerX = newTabButton->x + newTabButton->padding;
        newTabButton->innerY = newTabButton->y + newTabButton->padding;
}

/* Interface_newTabButton_redraw
 * Redraws the mew tab button.
 */
void Interface_newTabButton_redraw (void) {
        Interface_NewTabButton *newTabButton = &interface.tabBar.newTabButton;

        cairo_set_source_rgb(Window_context, TAB_BAR_COLOR);
        cairo_rectangle (
                Window_context,
                newTabButton->x,
                newTabButton->y,
                newTabButton->width,
                newTabButton->height - 1);
        cairo_fill(Window_context);

        if (
                Interface_Object_isHovered(newTabButton) ||
                Interface_Object_isClicked(newTabButton)
        ) {
                if (Interface_Object_isClicked(newTabButton)) {
                        cairo_set_source_rgb (
                                Window_context,
                                BUTTON_SYMBOL_CLICK_COLOR);
                } else {
                        cairo_set_source_rgb (
                                Window_context,
                                BUTTON_SYMBOL_HOVER_COLOR);
                }
                
                Interface_roundedRectangle (
                        newTabButton->innerX,
                        newTabButton->innerY,
                        newTabButton->innerWidth,
                        newTabButton->innerHeight, 4);
                cairo_fill(Window_context);
        }

        cairo_set_source_rgb(Window_context, BUTTON_SYMBOL_COLOR);
        double verticalX   = newTabButton->x + newTabButton->width / 2;
        double verticalY   = newTabButton->y + newTabButton->width / 2 - 6;
        double horizontalX = newTabButton->x + newTabButton->width / 2 - 6;
        double horizontalY = newTabButton->y + newTabButton->width / 2;
        cairo_set_line_width(Window_context, 1);
        cairo_move_to (
                Window_context,
                verticalX,
                verticalY);
        cairo_line_to (
                Window_context,
                verticalX,
                verticalY + 12);
                cairo_stroke(Window_context);
        cairo_move_to (
                Window_context,
                horizontalX,
                horizontalY);
        cairo_line_to (
                Window_context,
                horizontalX + 12,
                horizontalY);
                cairo_stroke(Window_context);
}

/* Interface_newTabButton_refresh
 * Recalculates the new tab button if it needs to be recalculated, and redraws
 * if it needs to redrawn.
 */
void Interface_newTabButton_refresh (void) {
        Interface_NewTabButton *newTabButton = &interface.tabBar.newTabButton;

        if (newTabButton->needsRecalculate) {
                Interface_newTabButton_recalculate();
                newTabButton->needsRecalculate = 0;
        }
        
        if (newTabButton->needsRedraw) {
                Interface_newTabButton_redraw();
                newTabButton->needsRedraw = 0;
        }
}

/* Interface_tabBar_getHoveredObject
 * Returns the object that the x and y coordinates are within.
 */
Interface_Object *Interface_tabBar_getHoveredObject (int x, int y) {
        Interface_TabBar *tabBar = &interface.tabBar;
        if (!Interface_Object_isWithinBounds(tabBar, x, y)) { return NULL; }
        
        if (Interface_Object_isWithinBounds(&tabBar->newTabButton, x, y)) {
                return TO_GENERIC(&tabBar->newTabButton);
        }

        Interface_Tab *tab = interface.tabBar.tabs;
        while (tab != NULL) {
                Interface_Object *object = TO_GENERIC (
                        Interface_Tab_getHoveredObject(tab, x, y));
                if (object != NULL) { return object; }
                
                tab = tab->next;
        }

        return TO_GENERIC(tabBar);
}
