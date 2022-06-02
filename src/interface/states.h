#pragma once

#include <cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "interface.h"
#include "window.h"
        
// these callbacks should generally fire when button/key combinations are
// pressed. it should not be possible to activate callbacks within from another
// callback. callbacks relating to management of visual structures that map to
// virtual structures (e.g. tabs) must update the virtual structure, and then
// instruct the Interface to update the visual structure (e.g. creating or
// deleting a tab and swapping the active edit buffer).
typedef struct {
        void (*onStart)     (void);
        void (*onNewTab)    (void);
        void (*onCloseTab)  (Interface_Tab *);
        void (*onSwitchTab) (Interface_Tab *);
} Interface_Callbacks;

typedef struct {
        FT_Library         freetypeHandle;
        FT_Face            freetypeFaceNormal;
        cairo_font_face_t *fontFaceNormal;
        
        double glyphHeight;
        double lineHeight;
        double glyphWidth;
        double capitalHeight;
} Interface_Fonts;

typedef struct {
        int x;
        int y;

        Window_State left;
        Window_State middle;
        Window_State right;

        size_t dragOriginRealX;
        size_t dragOriginRealY;

        Interface_Object *previousHoverObject;
        Interface_Object *hoverObject;
        Interface_Object *downObject;

        int inEditView;
        int dragOriginInEditView;
        int inTabBar;

        size_t cellX;
        size_t cellY;
} Interface_MouseState;

typedef struct {
        Window_State shift;
        Window_State ctrl;
        Window_State alt;
} Interface_ModKeyState;
