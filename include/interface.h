#pragma once

#include "safe-string.h"
#include "edit-buffer.h"
#include "error.h"

typedef struct Interface_Tab {
        int x;
        int y;
        int width;
        int height;

        String text;

        struct Interface_Tab *prev;
        struct Interface_Tab *next;
} Interface_Tab;

typedef struct {
        int x;
        int y;
        int width;
        int height;

        Interface_Tab *tabs;
        
} Interface_TabBar;

typedef struct {
        double x;
        double y;
        double width;
        double height;
        
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
} Interface_EditView;

typedef struct {
        int width;
        int height;

        Interface_TabBar   tabBar;
        Interface_EditView editView;
} Interface;

extern Interface interface;

Error Interface_run           (void);
void  Interface_setEditBuffer (EditBuffer *newEditBuffer);

// make event handlers for creating a new tab, clicking on a tab, etc
