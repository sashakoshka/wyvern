#include "module.h"
#include "options.h"

static void conditionallyRefresh (int);
static int  checkTabSelect       (void);

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
        // TODO: make generic setter method for this
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
        Interface_Object *object = Interface_getHoveredObject (
                interface.mouseState.x,
                interface.mouseState.y);

        // something is going to move or change - we need the cursor to be
        // visible
        interface.editView.text.cursorBlink = 1;

        int inCell =
                Interface_Object_isHovered(&interface.editView)      ||
                Interface_Object_isHovered(&interface.editView.text) ||
                Interface_Object_isHovered(&interface.editView.ruler);

        int dragOriginInCell =
                Interface_Object_isClicked(&interface.editView)      ||
                Interface_Object_isClicked(&interface.editView.text) ||
                Interface_Object_isClicked(&interface.editView.ruler);
        
        size_t cellX = 0;
        size_t cellY = 0;

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
                
                        interface.mouseState.downObject = NULL;
                }
        
                Interface_findMouseHoverCell (
                        interface.mouseState.x,
                        interface.mouseState.y,
                        &cellX, &cellY);
        
                interface.mouseState.left = state;
        
                // TODO: selection, etc.
                if (state == Window_State_on && inCell) {
                        size_t realX = 0;
                        size_t realY = 0;
                        TextDisplay_getRealCoords (
                                interface.editView.text.display,
                                cellX, cellY,
                                &realX, &realY);

                        interface.mouseState.dragOriginRealX = realX;
                        interface.mouseState.dragOriginRealY = realY;
                        
                        if (interface.modKeyState.ctrl) {
                                EditBuffer_addNewCursor (
                                        interface.editView.text.buffer,
                                        realX, realY);
                                Interface_Object_invalidateDrawing (
                                        &interface.editView.text);
                                break;
                        }

                        EditBuffer_clearExtraCursors (
                                interface.editView.text.buffer);
                        EditBuffer_Cursor_moveTo (
                                interface.editView.text.buffer->cursors,
                                realX, realY);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);

                        break;
                }

                if (state == Window_State_on) {
                        checkTabSelect();
                }
                break;
        
        case Window_MouseButton_middle:
                interface.mouseState.middle = state;
                // TODO: copy/paste
                break;
        
        case Window_MouseButton_right:
                interface.mouseState.right = state;
                // TODO: context menu
                break;
        
        case Window_MouseButton_scrollUp:
                if (state == Window_State_on && inCell) {
                        EditBuffer_scroll (
                                interface.editView.text.buffer,
                                Options_scrollSize * -1);
                        TextDisplay_grab(interface.editView.text.display);

                        if (
                                interface.mouseState.left &&
                                dragOriginInCell
                        ) {
                                Interface_updateTextSelection();
                                TextDisplay_grab (
                                        interface.editView.text.display);
                        }
        
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                }
                break;
                
        case Window_MouseButton_scrollDown:
                if (state == Window_State_on && inCell) {
                        EditBuffer_scroll (
                                interface.editView.text.buffer,
                                Options_scrollSize);
                        TextDisplay_grab(interface.editView.text.display);

                        if (
                                interface.mouseState.left &&
                                dragOriginInCell
                        ) {
                                Interface_updateTextSelection();
                                TextDisplay_grab (
                                        interface.editView.text.display);
                        }
                        
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
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

        Interface_Object *newHoverObject = Interface_getHoveredObject (
                interface.mouseState.x,
                interface.mouseState.y);

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

        if (
                interface.mouseState.left && (
                        Interface_Object_isClicked(&interface.editView)      ||
                        Interface_Object_isClicked(&interface.editView.text) ||
                        Interface_Object_isClicked(&interface.editView.ruler)
                )
        ) {
                Interface_updateTextSelection();
                Interface_Object_invalidateDrawing(&interface.editView.text);
        }
        
        conditionallyRefresh(render);
}

/* Interface_handleInterval
 * Fires every 500 milliseconds.
 */
void Interface_handleInterval (int render) {
        interface.editView.text.cursorBlink =
                !interface.editView.text.cursorBlink;
        // TODO: just invalidate the cursor blinking? possibly create a function
        // to invalidate the text display and call that every time the text
        // changes (that is, not here).
        Interface_Object_invalidateDrawing(&interface.editView.text);
        
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
                EditBuffer_clearExtraCursors(interface.editView.text.buffer);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                break;

        case WINDOW_KEY_ENTER:
        case WINDOW_KEY_PAD_ENTER:
                if (state == Window_State_on) {
                        EditBuffer_cursorsInsertRune (
                                interface.editView.text.buffer, '\n');
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                }
                break;

        case WINDOW_KEY_TAB:
                if (state == Window_State_on) {
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
                }
                break;
        
        case WINDOW_KEY_BACKSPACE:
                if (state == Window_State_on) {
                        EditBuffer_cursorsBackspaceRune (
                                interface.editView.text.buffer);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                }
                break;
        
        case WINDOW_KEY_DELETE:
                if (state == Window_State_on) {
                        EditBuffer_cursorsDeleteRune (
                                interface.editView.text.buffer);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
                }
                break;

        default:
                if (keySym >> 8 == 0 && state == Window_State_on) {
                        EditBuffer_cursorsInsertRune (
                                interface.editView.text.buffer, rune);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.ruler);
                        Interface_Object_invalidateDrawing (
                                &interface.editView.text);
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
}

/* Interface_handleKeyDown
 * Fires when the down arrow is pressed or released.
 */
void Interface_handleKeyDown (Window_State state) {
        if (state != Window_State_on) { return; }
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
                                interface.editView.text.buffer->cursors, 1);
                        EditBuffer_addNewCursor (
                                interface.editView.text.buffer, column, row);
                } else {
                        EditBuffer_cursorsSelectV (
                                interface.editView.text.buffer, 1);
                }
        } else {
                EditBuffer_cursorsMoveV(interface.editView.text.buffer, 1);
        }
        
        Interface_Object_invalidateDrawing(&interface.editView.text);
}

/* Interface_handleKeyLeft
 * Fires when the left arrow is pressed or released.
 */
void Interface_handleKeyLeft (Window_State state) {
        if (state != Window_State_on) { return; }
        if (interface.modKeyState.shift == Window_State_on) {
                EditBuffer_cursorsSelectH(interface.editView.text.buffer, -1);
        } else {
                EditBuffer_cursorsMoveH(interface.editView.text.buffer, -1);
        }
        
        Interface_Object_invalidateDrawing(&interface.editView.text);
}

/* Interface_handleKeyRight
 * Fires when the right arrow is pressed or released.
 */
void Interface_handleKeyRight (Window_State state) {
        if (state != Window_State_on) { return; }
        if (interface.modKeyState.shift == Window_State_on) {
                EditBuffer_cursorsSelectH(interface.editView.text.buffer, 1);
        } else {
                EditBuffer_cursorsMoveH(interface.editView.text.buffer, 1);
        }
        
        Interface_Object_invalidateDrawing(&interface.editView.text);
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
                        interface.callbacks.onSwitchTab(tab);
                        return 1;
                }
                
                tab = tab->next;
        }

        return 0;
}
