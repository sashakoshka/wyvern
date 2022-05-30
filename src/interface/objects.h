#pragma once

#include <limits.h>

#include "interface.h"
#include "states.h"
#include "text-display.h"

/* The interface is structured as follows:
 *
 *   ┌───────────────────────────────────────────────────────────┐
 *   │ Interface                                                 │
 *   │ ┌───────────────────────────────────────────────────────┐ │
 *   │ │ Interface_TabBar                                      │ │
 *   │ │ ┌───────────────┐ ┌───────────────┐ ┌───────────────┐ │ │
 *   │ │ │ Interface_Tab │ │ Interface_Tab │ │ Interface_Tab │ │ │
 *   │ │ └───────────────┘ └───────────────┘ └───────────────┘ │ │
 *   │ └───────────────────────────────────────────────────────┘ │
 *   │ ┌───────────────────────────────────────────────────────┐ │
 *   │ │ Interface_EditView                                    │ │
 *   │ │ ┌───┐ ┌─────────────────────────────────────────────┐ │ │
 *   │ │ │ I │ │ Interface_EditViewText                      │ │ │
 *   │ │ │ n │ │                                             │ │ │
 *   │ │ │ t │ │                                             │ │ │
 *   │ │ │ e │ │                                             │ │ │
 *   │ │ │ r │ │                                             │ │ │
 *   │ │ │ f │ │                                             │ │ │
 *   │ │ │ a │ │                                             │ │ │
 *   │ │ │ c │ │                                             │ │ │
 *   │ │ │ e │ │                                             │ │ │
 *   │ │ │ _ │ │                                             │ │ │
 *   │ │ │ E │ │                                             │ │ │
 *   │ │ │ d │ │                                             │ │ │
 *   │ │ │ i │ │                                             │ │ │
 *   │ │ │ t │ │                                             │ │ │
 *   │ │ │ V │ │                                             │ │ │
 *   │ │ │ i │ │                                             │ │ │
 *   │ │ │ e │ │                                             │ │ │
 *   │ │ │ w │ │                                             │ │ │
 *   │ │ │ R │ │                                             │ │ │
 *   │ │ │ u │ │                                             │ │ │
 *   │ │ │ l │ │                                             │ │ │
 *   │ │ │ e │ │                                             │ │ │
 *   │ │ │ r │ │                                             │ │ │
 *   │ │ └───┘ └─────────────────────────────────────────────┘ │ │
 *   │ └───────────────────────────────────────────────────────┘ │
 *   └───────────────────────────────────────────────────────────┘
 *
 * Each struct in this file represents a visual object, and holds information
 * that directly maps to, or is useful for, what is displayed on screen.
 */

#define INTERFACE_OBJECT \
        double x;        \
        double y;        \
        double width;    \
        double height;   \
        int    needsRecalculate; \
        int    needsRedraw;

struct Interface_Object {
        INTERFACE_OBJECT
};

struct Interface_Tab {
        INTERFACE_OBJECT

        double textX;
        double textY;

        char text[NAME_MAX + 1];

        double closeX;
        double closeY;
        double closeWidth;
        double closeHeight;

        struct Interface_Tab *previous;
        struct Interface_Tab *next;
};

struct Interface_TabBar {
        INTERFACE_OBJECT

        Interface_Tab *tabs;
        Interface_Tab *activeTab;
};

struct Interface_EditViewRuler {
        INTERFACE_OBJECT
};

struct Interface_EditViewText {
        INTERFACE_OBJECT
        
        int cursorBlink;
        
        EditBuffer  *buffer;
        TextDisplay *display;
};

struct Interface_EditView {
        INTERFACE_OBJECT
        
        double padding;

        double innerX;
        double innerY;
        double innerWidth;
        double innerHeight;

        Interface_EditViewRuler ruler;
        Interface_EditViewText  text;
};

struct Interface {
        INTERFACE_OBJECT
        
        int horizontal;

        Interface_TabBar   tabBar;
        Interface_EditView editView;
        
        Interface_Fonts       fonts;
        Interface_MouseState  mouseState;
        Interface_ModKeyState modKeyState;
        Interface_Callbacks   callbacks;
};
