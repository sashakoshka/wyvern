#include "module.h"
#include "utility.h"

static Interface_Tab *Interface_Tab_new  (void);
static void           Interface_Tab_free (Interface_Tab *);

/* Interface_tabBar_recalculate
 * Recalculates the position and size of the tab bar.
 */
void Interface_tabBar_recalculate (void) {
        interface.tabBar.x      = 0;
        interface.tabBar.y      = 0;
        interface.tabBar.height = 35;
        interface.tabBar.width  = interface.width;

        Interface_Tab *tab = interface.tabBar.tabs;
        while (tab != NULL) {
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
                double padding = (tab->height - capitalHeight) / 2;
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

                // onto next tab
                tab = tab->next;
        }
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

        Interface_Tab *tab = interface.tabBar.tabs;
        while (tab != NULL) {
                Interface_Tab_redraw(tab);
                tab = tab->next;
        }
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
void Interface_Tab_closeButtonRedraw (Interface_Tab *tab) {
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

/* Interface_TabBar_add
 * Appends a new tab to the linked list in the tab bar.
 */
Interface_Tab *Interface_TabBar_add (void) {
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

/* Interface_TabBar_delete
 * Removes an existing tab from the linked list in the tab bar.
 */
void Interface_TabBar_delete (Interface_Tab *tab) {
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
