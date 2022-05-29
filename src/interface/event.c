#include "module.h"
#include "options.h"

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
void Interface_handleRedraw (int width, int height) {
        Interface_recalculate(width, height);
        Interface_redraw();
}

/* Interface_handleMouseButton
 * Fires when a mouse button is pressed or released.
 */
void Interface_handleMouseButton (
        Window_MouseButton button,
        Window_State state
) {
        // something is going to move or change - we need the cursor to be
        // visible
        interface.editView.cursorBlink = 1;

        int inCell = HITBOX (
                interface.mouseState.x,
                interface.mouseState.y,
                interface.editView);
        
        size_t cellX = 0;
        size_t cellY = 0;

        switch (button) {
        case Window_MouseButton_left:
                Interface_findMouseHoverCell (
                        interface.mouseState.x,
                        interface.mouseState.y,
                        &cellX, &cellY);
                
                interface.mouseState.dragOriginX = interface.mouseState.x;
                interface.mouseState.dragOriginY = interface.mouseState.y;
                interface.mouseState.dragOriginInCell = inCell;
        
                interface.mouseState.left = state;
        
                // TODO: selection, etc.
                if (state == Window_State_on && inCell) {
                        size_t realX = 0;
                        size_t realY = 0;
                        TextDisplay_getRealCoords (
                                interface.editView.textDisplay,
                                cellX, cellY,
                                &realX, &realY);

                        interface.mouseState.dragOriginRealX = realX;
                        interface.mouseState.dragOriginRealY = realY;
                        
                        if (interface.modKeyState.ctrl) {
                                EditBuffer_addNewCursor (
                                        interface.editView.editBuffer,
                                        realX, realY);
                                Interface_editView_drawChars(1);
                                break;
                        }

                        EditBuffer_clearExtraCursors (
                                interface.editView.editBuffer);
                        EditBuffer_Cursor_moveTo (
                                interface.editView.editBuffer->cursors,
                                realX, realY);
                        Interface_editView_drawChars(1);
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
                                interface.editView.editBuffer,
                                Options_scrollSize * -1);
                        TextDisplay_grab(interface.editView.textDisplay);

                        if (
                                interface.mouseState.left &&
                                interface.mouseState.dragOriginInCell
                        ) {
                                Interface_updateTextSelection();
                                TextDisplay_grab (
                                        interface.editView.textDisplay);
                        }
        
                        Interface_editView_drawRuler();
                        Interface_editView_drawChars(0);
                }
                break;
                
        case Window_MouseButton_scrollDown:
                if (state == Window_State_on && inCell) {
                        EditBuffer_scroll (
                                interface.editView.editBuffer,
                                Options_scrollSize);
                        TextDisplay_grab(interface.editView.textDisplay);

                        if (
                                interface.mouseState.left &&
                                interface.mouseState.dragOriginInCell
                        ) {
                                Interface_updateTextSelection();
                                TextDisplay_grab (
                                        interface.editView.textDisplay);
                        }
                        
                        Interface_editView_drawRuler();
                        Interface_editView_drawChars(0);
                }
                break;
        }
}

/* Interface_handleMouseMove
 * Fires when the mouse is moved.
 */
void Interface_handleMouseMove (int x, int y) {
        interface.mouseState.x = x;
        interface.mouseState.y = y;

        if (
                interface.mouseState.left &&
                interface.mouseState.dragOriginInCell
        ) {
                Interface_updateTextSelection();
                Interface_editView_drawChars(1);
        }
}

/* Interface_handleInterval
 * Fires every 500 milliseconds.
 */
void Interface_handleInterval (void) {
        Interface_editView_drawChars(0);
        interface.editView.cursorBlink = !interface.editView.cursorBlink;
}

/* Interface_handleKey
 * Fires when a key is pressed or released.
 */
void Interface_handleKey (Window_KeySym keySym, Rune rune, Window_State state) {
        // if (state == Window_State_on) {
                // printf("%lx\n", keySym);
        // }

        // something is going to move or change - we need the cursor to be
        // visible
        interface.editView.cursorBlink = 1;
        
        switch (keySym) {
        case WINDOW_KEY_SHIFT: interface.modKeyState.shift = state; return;
        case WINDOW_KEY_CTRL:  interface.modKeyState.ctrl  = state; return;
        case WINDOW_KEY_ALT:   interface.modKeyState.alt   = state; return;

        case WINDOW_KEY_UP:    Interface_handleKeyUp(state);    return;
        case WINDOW_KEY_DOWN:  Interface_handleKeyDown(state);  return;
        case WINDOW_KEY_LEFT:  Interface_handleKeyLeft(state);  return;
        case WINDOW_KEY_RIGHT: Interface_handleKeyRight(state); return;

        case WINDOW_KEY_ESCAPE:
                EditBuffer_clearExtraCursors(interface.editView.editBuffer);
                Interface_editView_drawChars(1);
                break;

        case WINDOW_KEY_ENTER:
        case WINDOW_KEY_PAD_ENTER:
                if (state == Window_State_on) {
                        EditBuffer_cursorsInsertRune (
                                interface.editView.editBuffer, '\n');
                        Interface_editView_drawChars(1);
                        Interface_editView_drawRuler();
                }
                return;

        case WINDOW_KEY_TAB:
                if (state == Window_State_on) {
                        EditBuffer_cursorsInsertRune (
                                interface.editView.editBuffer, '\t');
                        if (!Options_tabsToSpaces) {
                                EditBuffer_cursorsMoveH (
                                        interface.editView.editBuffer, 1);
                        }
                        Interface_editView_drawChars(1);
                        Interface_editView_drawRuler();
                }
                return;
        
        case WINDOW_KEY_BACKSPACE:
                if (state == Window_State_on) {
                        EditBuffer_cursorsBackspaceRune (
                                interface.editView.editBuffer);
                        Interface_editView_drawChars(1);
                        Interface_editView_drawRuler();
                }
                return;
        
        case WINDOW_KEY_DELETE:
                if (state == Window_State_on) {
                        EditBuffer_cursorsDeleteRune (
                                interface.editView.editBuffer);
                        Interface_editView_drawChars(1);
                        Interface_editView_drawRuler();
                }
                return;
        }

        if (keySym >> 8 == 0 && state == Window_State_on) {
                EditBuffer_cursorsInsertRune (
                        interface.editView.editBuffer, rune);
                Interface_editView_drawChars(1);
        }
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
                                editBuffer->cursors[0].column;
                        size_t row =
                                interface.
                                editView.
                                editBuffer->cursors[0].row;
                        EditBuffer_Cursor_moveV (
                                interface.editView.editBuffer->cursors, -1);
                        EditBuffer_addNewCursor (
                                interface.editView.editBuffer, column, row);
                } else {
                        EditBuffer_cursorsSelectV (
                                interface.editView.editBuffer, -1);
                }
        } else {
                EditBuffer_cursorsMoveV(interface.editView.editBuffer, -1);
        }
        Interface_editView_drawChars(1);
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
                                editBuffer->cursors[0].column;
                        size_t row =
                                interface.
                                editView.
                                editBuffer->cursors[0].row;
                        EditBuffer_Cursor_moveV (
                                interface.editView.editBuffer->cursors, 1);
                        EditBuffer_addNewCursor (
                                interface.editView.editBuffer, column, row);
                } else {
                        EditBuffer_cursorsSelectV (
                                interface.editView.editBuffer, 1);
                }
        } else {
                EditBuffer_cursorsMoveV(interface.editView.editBuffer, 1);
        }
        Interface_editView_drawChars(1);
}

/* Interface_handleKeyLeft
 * Fires when the left arrow is pressed or released.
 */
void Interface_handleKeyLeft (Window_State state) {
        if (state != Window_State_on) { return; }
        if (interface.modKeyState.shift == Window_State_on) {
                EditBuffer_cursorsSelectH(interface.editView.editBuffer, -1);
        } else {
                EditBuffer_cursorsMoveH(interface.editView.editBuffer, -1);
        }
        Interface_editView_drawChars(1);
}

/* Interface_handleKeyRight
 * Fires when the right arrow is pressed or released.
 */
void Interface_handleKeyRight (Window_State state) {
        if (state != Window_State_on) { return; }
        if (interface.modKeyState.shift == Window_State_on) {
                EditBuffer_cursorsSelectH(interface.editView.editBuffer, 1);
        } else {
                EditBuffer_cursorsMoveH(interface.editView.editBuffer, 1);
        }
        Interface_editView_drawChars(1);
}
