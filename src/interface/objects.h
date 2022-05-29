#pragma once

#include <limits.h>

#include "interface.h"
#include "states.h"
#include "text-display.h"

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

struct Interface_EditView {
        INTERFACE_OBJECT
        
        double padding;
        double rulerWidth;

        double innerX;
        double innerY;
        double innerWidth;
        double innerHeight;

        double textX;
        double textY;
        double textWidth;
        double textHeight;

        int cursorBlink;
        
        EditBuffer  *editBuffer;
        TextDisplay *textDisplay;
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
