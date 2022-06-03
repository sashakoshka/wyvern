#include "module.h"
#include "options.h"

// im a twisted fucking cycle path
#define OR ||
#define BUFFER_EXISTS (interface.editView.text.buffer != NULL)

static void conditionallyRefresh (int);
static void updateHoverObject    (void);
static int  checkTabSelect       (void);
static int  checkNewTab          (void);
static int  checkCloseTab        (void);
static int  checkMiddleCloseTab  (void);
static int  checkTextClick       (void);

/* Interface_onStart
 * Sets the function to be called when Interface finishes starting up.
 */
void Interface_onStart (void (*callback) (void)) {
        interface.callbacks.onStart = callback;
}

/* Interface_onNewTab
 * Sets the function to be called when the new tab button is pressed.
 */
void Interface_onNewTab (void (*callback) (void)) {
        interface.callbacks.onNewTab = callback;
}

/* Interface_onCloseTab
 * Sets the function to be called when the close button is pressed on a tab.
 */
void Interface_onCloseTab (void (*callback) (Interface_Tab *)) {
        interface.callbacks.onCloseTab = callback;
}

/* Interface_onSwitchTab
 * Sets the function to be called when the user selects a tab.
 */
void Interface_onSwitchTab (void (*callback) (Interface_Tab *)) {
        interface.callbacks.onSwitchTab = callback;
}

/* Interface_handleRedraw
 * Fires when the screen needs to be redrawn.
 */
void Interface_handleRedraw (int render, int width, int height) {
        // TODO: make generic setter method for this??
        interface.width  = width;
        interface.height = height;
        
        Interface_invalidateLayout();
        Interface_invalidateDrawing();

        conditionallyRefresh(render);
}

/* Interface_handleMouseButton
 * Fires when a mouse button is pressed or released.
 */
void Interface_handleMouseButton (
        int                render, 
        Window_MouseButton button,
        Window_State state
) {
        updateHoverObject();
        Interface_Object *object = interface.mouseState.hoverObject;

        // something is going to move or change - we need the cursor to be
        // visible
        interface.editView.text.cursorBlink = 1;

        switch (button) {
        case Window_MouseButton_left:
                if (state == Window_State_on) {
                        if (object != NULL && object->redrawOnMouseButton) {
                                object->needsRedraw = 1;
                        }

                        interface.mouseState.downObject = object;
                } else {
                        object = interface.mouseState.downObject;
                        if (object != NULL && object->redrawOnMouseButton) {
                                object->needsRedraw = 1;
                        }
                }
        
                interface.mouseState.left = state;
        
                if (state == Window_State_on) {
                        checkTextClick() OR
                        checkTabSelect();
                } else {
                        checkNewTab() OR
                        checkCloseTab();
                }

                if (state == Window_State_off) {
                        interface.mouseState.downObject = NULL;
                }
                break;
        
        case Window_MouseButton_middle:
                interface.mouseState.middle = state;
                if (state == Window_State_on) {
                        checkMiddleCloseTab();
                }
                break;
        
        case Window_MouseButton_right:
                interface.mouseState.right = state;
                // TODO: context menu
                break;
        
        case Window_MouseButton_scrollUp:
                if (state == Window_State_off) { break; }
                
                if (interface.mouseState.inEditView && BUFFER_EXISTS) {
                        EditBuffer_scroll (
                                interface.editView.text.buffer,
                                Options_scrollSize * -1);
                        TextDisplay_grab(interface.editView.text.display);

                        if (
                                interface.mouseState.left &&
                                interface.mouseState.dragOriginInEditView
                        ) {
                                Interface_updateTextSelection();
                                TextDisplay_grab (
                                        interface.editView.text.display);
                        }
        
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                        Interface_editViewText_invalidateText();
                } else if (interface.mouseState.inTabBar) {
                        Interface_tabBar_scroll(-32);
                }
                break;
                
        case Window_MouseButton_scrollDown:
                if (state == Window_State_off) { break; }
                
                if (interface.mouseState.inEditView && BUFFER_EXISTS) {
                        EditBuffer_scroll (
                                interface.editView.text.buffer,
                                Options_scrollSize);
                        TextDisplay_grab(interface.editView.text.display);

                        if (
                                interface.mouseState.left &&
                                interface.mouseState.dragOriginInEditView
                        ) {
                                Interface_updateTextSelection();
                                TextDisplay_grab (
                                        interface.editView.text.display);
                        }
                        
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                        Interface_editViewText_invalidateText();
                } else if (interface.mouseState.inTabBar) {
                        Interface_tabBar_scroll(32);
                }
                break;
        }
        
        conditionallyRefresh(render);
}

/* Interface_handleMouseMove
 * Fires when the mouse is moved.
 */
void Interface_handleMouseMove (int render, int x, int y) {
        interface.mouseState.x = x;
        interface.mouseState.y = y;

        updateHoverObject();

        if (
                interface.mouseState.left && 
                interface.mouseState.dragOriginInEditView &&
                BUFFER_EXISTS
        ) {
                Interface_updateTextSelection();
                Interface_Object_invalidateDrawing(&interface.editView.text);
                Interface_editViewText_invalidateText();
        }
        
        conditionallyRefresh(render);
}

/* updateHoverObject
 * Updates various information about what the mouse is currently hovering over.
 */
static void updateHoverObject (void) {
        Interface_Object *newHoverObject = Interface_getHoveredObject (
                interface.mouseState.x,
                interface.mouseState.y);

        interface.mouseState.inTabBar = Interface_Object_isWithinBounds (
                &interface.tabBar,
                interface.mouseState.x,
                interface.mouseState.y);

        interface.mouseState.inEditView = Interface_Object_isWithinBounds (
                &interface.editView,
                interface.mouseState.x,
                interface.mouseState.y);

        Interface_findMouseHoverCell (
                interface.mouseState.x,
                interface.mouseState.y,
                &interface.mouseState.cellX, &interface.mouseState.cellY);

        if (interface.mouseState.hoverObject != newHoverObject) {
                if (
                        interface.mouseState.hoverObject != NULL &&
                        interface.mouseState.hoverObject->redrawOnHover
                ) {
                        interface.mouseState.hoverObject->needsRedraw = 1;
                }
                
                if (
                        newHoverObject != NULL &&
                        newHoverObject->redrawOnHover
                ) {
                        newHoverObject->needsRedraw = 1;
                }
        }
        
        interface.mouseState.previousHoverObject =
                interface.mouseState.hoverObject;
        interface.mouseState.hoverObject = newHoverObject;
}

/* Interface_handleInterval
 * Fires every 500 milliseconds.
 */
void Interface_handleInterval (int render) {
        interface.editView.text.cursorBlink =
                !interface.editView.text.cursorBlink;

        // we don't need to blink the cursor if the buffer doesn't exist
        if (BUFFER_EXISTS) {
                Interface_Object_invalidateDrawing(&interface.editView.text);
        }
        
        conditionallyRefresh(render);
}

/* Interface_handleKey
 * Fires when a key is pressed or released.
 */
void Interface_handleKey (
        int           render,
        Window_KeySym keySym,
        Rune          rune,
        Window_State  state) {
        // if (state == Window_State_on) {
                // printf("%lx\n", keySym);
        // }

        // something is going to move or change - we need the cursor to be
        // visible
        interface.editView.text.cursorBlink = 1;
        
        switch (keySym) {
        case WINDOW_KEY_SHIFT: interface.modKeyState.shift = state; break;
        case WINDOW_KEY_CTRL:  interface.modKeyState.ctrl  = state; break;
        case WINDOW_KEY_ALT:   interface.modKeyState.alt   = state; break;

        case WINDOW_KEY_UP:    Interface_handleKeyUp(state);    break;
        case WINDOW_KEY_DOWN:  Interface_handleKeyDown(state);  break;
        case WINDOW_KEY_LEFT:  Interface_handleKeyLeft(state);  break;
        case WINDOW_KEY_RIGHT: Interface_handleKeyRight(state); break;

        case WINDOW_KEY_ESCAPE:
                if (BUFFER_EXISTS) {
                        EditBuffer_clearExtraCursors (
                                interface.editView.text.buffer);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                        Interface_editViewText_invalidateText();
                }
                break;

        case WINDOW_KEY_ENTER:
        case WINDOW_KEY_PAD_ENTER:
                if (state == Window_State_on && BUFFER_EXISTS) {
                        EditBuffer_cursorsInsertRune (
                                interface.editView.text.buffer, '\n');
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                        Interface_editViewText_invalidateText();
                }
                break;

        case WINDOW_KEY_TAB:
                if (state == Window_State_on && BUFFER_EXISTS) {
                        EditBuffer_cursorsInsertRune (
                                interface.editView.text.buffer, '\t');
                        if (!Options_tabsToSpaces) {
                                EditBuffer_cursorsMoveH (
                                        interface.editView.text.buffer, 1);
                        }
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                        Interface_editViewText_invalidateText();
                }
                break;
        
        case WINDOW_KEY_BACKSPACE:
                if (state == Window_State_on && BUFFER_EXISTS) {
                        EditBuffer_cursorsBackspaceRune (
                                interface.editView.text.buffer);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                        Interface_editViewText_invalidateText();
                }
                break;
        
        case WINDOW_KEY_DELETE:
                if (state == Window_State_on && BUFFER_EXISTS) {
                        EditBuffer_cursorsDeleteRune (
                                interface.editView.text.buffer);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                        Interface_editViewText_invalidateText();
                }
                break;

        default:
                if (
                        keySym >> 8 == 0 && state == Window_State_on &&
                        BUFFER_EXISTS
                ) {
                        EditBuffer_cursorsInsertRune (
                                interface.editView.text.buffer, rune);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                        Interface_editViewText_invalidateText();
                }
                break;
        }
        
        conditionallyRefresh(render);
}

/* Interface_handleKeyUp
 * Fires when the up arrow is pressed or released.
 */
void Interface_handleKeyUp (Window_State state) {
        if (state != Window_State_on) { return; }

        if (BUFFER_EXISTS) {
                if (interface.modKeyState.shift == Window_State_on) {
                        if (interface.modKeyState.alt == Window_State_on) {
                                size_t column =
                                        interface.
                                        editView.
                                        text.buffer->cursors[0].column;
                                size_t row =
                                        interface.
                                        editView.
                                        text.buffer->cursors[0].row;
                                EditBuffer_Cursor_moveV (
                                        interface.editView.text.buffer->cursors, -1);
                                EditBuffer_addNewCursor (
                                        interface.editView.text.buffer, column, row);
                        } else {
                                EditBuffer_cursorsSelectV (
                                        interface.editView.text.buffer, -1);
                        }
                } else {
                        EditBuffer_cursorsMoveV(interface.editView.text.buffer, -1);
                }
                
                Interface_Object_invalidateDrawing(&interface.editView.text);
                Interface_editViewText_invalidateText();
        }
}

/* Interface_handleKeyDown
 * Fires when the down arrow is pressed or released.
 */
void Interface_handleKeyDown (Window_State state) {
        if (state != Window_State_on) { return; }

        if (BUFFER_EXISTS) {
                if (interface.modKeyState.shift == Window_State_on) {
                        if (interface.modKeyState.alt == Window_State_on) {
                                size_t column =
                                        interface.
                                        editView.
                                        text.buffer->cursors[0].column;
                                size_t row =
                                        interface.
                                        editView.
                                        text.buffer->cursors[0].row;
                                EditBuffer_Cursor_moveV (
                                        interface.editView.text.buffer->cursors,
                                        1);
                                EditBuffer_addNewCursor (
                                        interface.editView.text.buffer, column,
                                        row);
                        } else {
                                EditBuffer_cursorsSelectV (
                                        interface.editView.text.buffer, 1);
                        }
                } else {
                        EditBuffer_cursorsMoveV (
                                interface.editView.text.buffer, 1);
                }
                
                Interface_Object_invalidateDrawing(&interface.editView.text);
                Interface_editViewText_invalidateText();
        }
}

/* Interface_handleKeyLeft
 * Fires when the left arrow is pressed or released.
 */
void Interface_handleKeyLeft (Window_State state) {
        if (state != Window_State_on) { return; }
        
        if (BUFFER_EXISTS) {
                if (interface.modKeyState.shift == Window_State_on) {
                        EditBuffer_cursorsSelectH (
                                interface.editView.text.buffer, -1);
                } else {
                        EditBuffer_cursorsMoveH (
                                interface.editView.text.buffer, -1);
                }
                
                Interface_Object_invalidateDrawing(&interface.editView.text);
                Interface_editViewText_invalidateText();
        }
}

/* Interface_handleKeyRight
 * Fires when the right arrow is pressed or released.
 */
void Interface_handleKeyRight (Window_State state) {
        if (state != Window_State_on) { return; }

        if (BUFFER_EXISTS) {
                if (interface.modKeyState.shift == Window_State_on) {
                        EditBuffer_cursorsSelectH (
                                interface.editView.text.buffer, 1);
                } else {
                        EditBuffer_cursorsMoveH (
                                interface.editView.text.buffer, 1);
                }
                
                Interface_Object_invalidateDrawing(&interface.editView.text);
                Interface_editViewText_invalidateText();
        }
}

/* conditionallyRefresh
 * Refreshes the entire interface if the render parameter is 1.
 */
static void conditionallyRefresh (int render) {
        if (render) {
                Interface_refresh();
        }
}

/* checkTabSelect
 * Checks if a tab has been selected, and if it has, fires the onTabSelect
 * event. If it happened, returns 1. Assumes the left mouse button has been
 * pressed.
 */
static int checkTabSelect (void) {
        Interface_Tab *tab = interface.tabBar.tabs;
        while (tab != NULL) {
                if (Interface_Object_isHovered(tab)) {
                        Interface_Tab_invalidateDrawing (
                                interface.tabBar.activeTab);
                        Interface_Tab_invalidateDrawing(tab);

                        if (interface.callbacks.onSwitchTab != NULL) {
                                interface.callbacks.onSwitchTab(tab);
                        }
                        return 1;
                }
                
                tab = tab->next;
        }

        return 0;
}

/* checkNewTab
 * Checks if a tab has been selected, and if it has, fires the onNewTab event.
 * If it happened, returns 1. Assumes the left mouse button has been released.
 */
static int checkNewTab (void) {
        Interface_NewTabButton *newTabButton = &interface.tabBar.newTabButton;
        
        if (
                Interface_Object_isHovered(newTabButton) &&
                Interface_Object_isClicked(newTabButton)
        ) {
                if (interface.callbacks.onNewTab != NULL) {
                        interface.callbacks.onNewTab();
                }
                return 1;
        }

        return 0;
}

/* checkCloseTab
 * Checks if a tab's close button has been pressed, and if it has, fires the
 * onCloseTab event. If it happened, returns 1. Assumes the left mouse button
 * has been released.
 */
static int checkCloseTab (void) {
        Interface_Tab *tab = interface.tabBar.tabs;
        while (tab != NULL) {
                if (
                        Interface_Object_isHovered(&tab->closeButton) &&
                        Interface_Object_isClicked(&tab->closeButton)
                ) {
                        Interface_tabBar_invalidateLayout();
                        if (interface.callbacks.onCloseTab != NULL) {
                                interface.callbacks.onCloseTab(tab);
                        }
                        return 1;
                }
                
                tab = tab->next;
        }

        return 0;
}

/* checkMiddleCloseTab
 * Checks if a tab has been closed with the middle mouse button, and if it has,
 * fires the onCloseTab event. If it happened, returns 1. Assumes the middle
 * mouse button has been pressed.
 */
static int checkMiddleCloseTab (void) {
        Interface_Tab *tab = interface.tabBar.tabs;
        while (tab != NULL) {
                if (
                        Interface_Object_isHovered(tab) ||
                        Interface_Object_isHovered(&tab->closeButton)
                ) {
                        Interface_tabBar_invalidateLayout();

                        if (interface.callbacks.onCloseTab != NULL) {
                                interface.callbacks.onCloseTab(tab);
                        }
                        return 1;
                }
                
                tab = tab->next;
        }

        return 0;
}

/* checkTextClick
 * Handles mouse down interaction with the text view. If something was handled,
 * returns 1. Assumes the left mouse button has been pressed.
 */
static int checkTextClick (void) {
        interface.mouseState.dragOriginInEditView =
                interface.mouseState.inEditView;
        
        if (interface.mouseState.inEditView && BUFFER_EXISTS) {
                size_t realX = 0;
                size_t realY = 0;
                TextDisplay_getRealCoords (
                        interface.editView.text.display,
                        interface.mouseState.cellX,
                        interface.mouseState.cellY,
                        &realX, &realY);

                interface.mouseState.dragOriginRealX = realX;
                interface.mouseState.dragOriginRealY = realY;
                
                if (interface.modKeyState.ctrl) {
                        EditBuffer_addNewCursor (
                                interface.editView.text.buffer,
                                realX, realY);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                        Interface_editViewText_invalidateText();
                } else {
                        EditBuffer_clearExtraCursors (
                                interface.editView.text.buffer);
                        EditBuffer_Cursor_moveTo (
                                interface.editView.text.buffer->cursors,
                                realX, realY);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                        Interface_editViewText_invalidateText();
                }

                return 1;
        }

        return 0;
}
