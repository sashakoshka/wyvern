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
                tab->x      = 0 + interface.tabBar.x;
                tab->y      = 0 + interface.tabBar.y;
                tab->width  = 96;
                tab->height = interface.tabBar.height - 1;

                if (tab->previous) {
                        tab->x += tab->previous->x;
                }
                
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
        cairo_set_source_rgb(Window_context, INACTIVE_TAB_COLOR);
        cairo_rectangle (
                Window_context,
                tab->x,
                tab->y,
                tab->width,
                tab->height);
        cairo_fill(Window_context);
        
        cairo_set_source_rgb(Window_context, TEXT_COLOR);
        Interface_fontNormal();
        cairo_move_to (
                Window_context,
                tab->x, tab->y + 20);
        cairo_show_text(Window_context, tab->text);        
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
