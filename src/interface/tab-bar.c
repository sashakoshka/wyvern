#include "module.h"
#include "utility.h"

static Interface_Tab *Interface_Tab_new               (void);
static void           Interface_Tab_free              (Interface_Tab *);
static void           Interface_Tab_closeButtonRedraw (Interface_Tab *);

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
Interface_Tab *Interface_tabBar_add (void) {
        Interface_Tab *tab = Interface_Tab_new();

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
        interface.tabBar.activeTab = tab;
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

/* Interface_Tab_redraw
 * Recalculates a single tab.
 */
void Interface_Tab_recalculate (Interface_Tab *tab) {
        // tab position
        tab->x = 0 + interface.tabBar.x;
        tab->y = 0 + interface.tabBar.y;

        if (tab->previous != NULL) {
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
        tab->closeWidth    = 8;
        tab->closeHeight   = tab->closeWidth;
        double closeMargin = (tab->height - tab->closeHeight) / 2;

        // tab width
        tab->width =
                padding +
                textExtents.width +
                closeMargin +
                tab->closeWidth +
                closeMargin;

        // close button position
        tab->closeX =
                tab->x + tab->width -
                closeMargin -
                tab->closeWidth;
        tab->closeY = tab->y + closeMargin;
}

/* Interface_Tab_redraw
 * Redraws a single tab.
 */
void Interface_Tab_redraw (Interface_Tab *tab) {
        // tab background
        if (tab == interface.tabBar.activeTab) {
                cairo_set_source_rgb(Window_context, ACTIVE_TAB_COLOR);
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

        // close button
        Interface_Tab_closeButtonRedraw(tab);
}

/* Interface_Tab_closeButtonRedraw
 * Redraws the tab's close button.
 */
static void Interface_Tab_closeButtonRedraw (Interface_Tab *tab) {
        if (tab == interface.tabBar.activeTab) {
                cairo_set_source_rgb(Window_context, ACTIVE_TAB_COLOR);
        } else {
                cairo_set_source_rgb(Window_context, INACTIVE_TAB_COLOR);
        }
        cairo_rectangle (
                Window_context,
                tab->closeX,
                tab->closeY,
                tab->closeWidth,
                tab->closeHeight);
        cairo_fill(Window_context);
        
        cairo_set_source_rgb(Window_context, CLOSE_BUTTON_COLOR);
        double nearX = tab->closeX + 1;
        double nearY = tab->closeY + 1;
        double farX  = tab->closeX + tab->closeWidth  - 1;
        double farY  = tab->closeY + tab->closeHeight - 1;
        cairo_set_line_width(Window_context, 2);
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
}

/* Interface_Tab_setText
 * Sets the text that will be displayed as the tab title.
 */
void Interface_Tab_setText (Interface_Tab *tab, const char *text) {
        Utility_copyCString(tab->text, text, NAME_MAX);
}


/* Interface_Tab_new
 * Allocates and returns new tab.
 */
static Interface_Tab *Interface_Tab_new (void) {
        Interface_Tab *tab = calloc(1, sizeof(*tab));
        return tab;
}

/* Interface_Tab_free
 * Frees a tab.
 */
static void Interface_Tab_free (Interface_Tab *tab) {
        free(tab);
}

/* Interface_Tab_invalidateLayout
 * Invalidates the layout of a single tab.
 */
void Interface_Tab_invalidateLayout (Interface_Tab *tab) {
        tab->needsRedraw = 1;
        tab->needsRecalculate = 1;
}

/* Interface_Tab_invalidateDrawing
 * Invalidates the drawing of a single tab.
 */
void Interface_Tab_invalidateDrawing (Interface_Tab *tab) {
        tab->needsRedraw = 1;
}

/* Interface_Tab_refresh
 * Recalculates the tab bar if it needs to be recalculated, and redraws if it
 * needs to redrawn.
 */
void Interface_Tab_refresh (Interface_Tab *tab) {
        if (tab->needsRecalculate == 1) {
                Interface_Tab_recalculate(tab);
                tab->needsRecalculate = 0;
        }
        
        if (tab->needsRedraw == 1) {
                Interface_Tab_redraw(tab);
                tab->needsRedraw = 0;
        }
}

/* Interface_newTabButton_recalculate
 * Recalculates the size amd position of the new tab button.
 */
void Interface_newTabButton_recalculate (void) {
        Interface_TabBar       *tabBar       = &interface.tabBar;
        Interface_NewTabButton *newTabButton = &tabBar->newTabButton;

        newTabButton->height = tabBar->height - 8;
        newTabButton->width  = newTabButton->height;
        newTabButton->x =
                tabBar->x +
                tabBar->width -
                newTabButton->width - 4;
        newTabButton->y = tabBar->y + 4;
}

/* Interface_newTabButton_redraw
 * Redraws the mew tab button.
 */
void Interface_newTabButton_redraw (void) {
        Interface_NewTabButton *newTabButton = &interface.tabBar.newTabButton;

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
                        newTabButton->x,
                        newTabButton->y,
                        newTabButton->width,
                        newTabButton->height, 4);
                cairo_fill(Window_context);
        } else {
                cairo_set_source_rgb(Window_context, TAB_BAR_COLOR);
                cairo_rectangle (
                        Window_context,
                        newTabButton->x,
                        newTabButton->y,
                        newTabButton->width,
                        newTabButton->height);
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

        if (newTabButton->needsRecalculate == 1) {
                Interface_newTabButton_recalculate();
                newTabButton->needsRecalculate = 0;
        }
        
        if (newTabButton->needsRedraw == 1) {
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
                if (Interface_Object_isWithinBounds(tab, x, y)) {
                        return TO_GENERIC(tab);
                }
                tab = tab->next;
        }

        return TO_GENERIC(tabBar);
}
