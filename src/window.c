#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <cairo.h>
#include <cairo-xlib.h>

#include <stdio.h>
#include <stdlib.h>

#include "window.h"

cairo_surface_t *Window_surface = { 0 };
cairo_t         *Window_context = { 0 };

static int width  = 640;
static int height = 480;
static int listening = 0;
static int started   = 0;

static Window   window  = { 0 };
static Display *display = { 0 };
static int      screen  = { 0 };

static Atom windowDeleteEvent;

static struct {
        void (*onRedraw)      (int, int);
        void (*onMouseButton) (Window_MouseButton, Window_State);
        void (*onMouseMove)   (int, int);
} callbacks = { 0 };

static Error respondToEvent       (XEvent);
static Error respondToEventButton (unsigned int, Window_State);

/* Window_start
 * Opens the window and sets up the cairo rendering context. THe window will
 * remain hidden uneil Window_show is called.
 */
Error Window_start () {
        display = XOpenDisplay(NULL);
        if (display == NULL) { return Error_cantOpenDisplay; }

        screen = DefaultScreen(display);
        window = XCreateSimpleWindow (
                display, DefaultRootWindow(display),
                0, 0, (unsigned)width, (unsigned)height,
                0, 0, 0);

        XSelectInput (
                display, window,
                PointerMotionMask |
                ButtonPressMask | ButtonReleaseMask   |
                KeyPressMask    | KeyReleaseMask      |
                ExposureMask    | StructureNotifyMask);

        windowDeleteEvent = XInternAtom(display, "WM_DELETE_WINDOW", True);
        XSetWMProtocols(display, window, &windowDeleteEvent, 1);

        Window_surface = cairo_xlib_surface_create (
                display, window,
                DefaultVisual(display, screen),
                width, height);

        cairo_xlib_surface_set_size (
                Window_surface,
                width, height);

        Window_context = cairo_create(Window_surface);

        started = 1;
        return Error_none;
}

/* Window_show
 * Maps the window into the screen. This function should be called after basic
 * setup has completed.
 */
Error Window_show (void) {
        int status = XMapWindow(display, window);
        if (status != 1) { return Error_cantMapWindow; }

        return Error_none;
}

/* Window_listen
 * Blocking event loop. This function will return when the application exits,
 * from within an event handler or in response to a close request from the
 * window manager.
 */
Error Window_listen (void) {
        listening = 1;
        while (listening) {
                XEvent event; 
                XNextEvent(display, &event);
                Error err = respondToEvent(event);
                if (err) { return err; }
        }

        return Error_none;
}

/* respondToEvent
 * Handle a single event from the Xlib event loop in Window_listen.
 */
static Error respondToEvent (XEvent event) {
        Error err;
        
        switch (event.type) {
        case ButtonPress:
                err = respondToEventButton (
                        event.xbutton.button,
                        Window_State_on);
                if (err) { return err; }
                break;
        
        case ButtonRelease:
                err = respondToEventButton (
                        event.xbutton.button,
                        Window_State_off);
                if (err) { return err; }
                break;

        case MotionNotify: ;
                int mouseX;
                int mouseY;
                // seriously xlib? i just want the mose position.
                int          garbageI;
                unsigned int garbageU;
                Window       garbageW;
                XQueryPointer (
                        display, window,
                        &garbageW, &garbageW, &garbageI, &garbageI,
                        &mouseX, &mouseY,
                        &garbageU);
                        
                if (callbacks.onMouseMove == NULL) {
                        return Error_nullCallback;
                }
                callbacks.onMouseMove(mouseX, mouseY);
                
                break;

        case Expose:
                if (callbacks.onRedraw == NULL) {
                        return Error_nullCallback;
                }

                cairo_push_group(Window_context);
                cairo_paint(Window_context);
                callbacks.onRedraw(width, height);
                cairo_pop_group_to_source(Window_context);
                cairo_paint(Window_context);        
                cairo_surface_flush(Window_surface);
                break;

        case ConfigureNotify: ;
                int newWidth  = event.xconfigure.width;
                int newHeight = event.xconfigure.height;
                if (newWidth == width && newHeight == height) { break; }

                width  = newWidth;
                height = newHeight;

                cairo_xlib_surface_set_size (
                        Window_surface,
                        width, height);
                break;
        
        case ClientMessage:
                Window_stop();
                break;
        }
        return Error_none;
}

/* respondToEventButton
 * Respond to a single mouse button event.
 */
static Error respondToEventButton (
        unsigned int button,
        Window_State state
) {
        if (callbacks.onMouseButton == NULL) {
                return Error_nullCallback;
        }
        
        switch (button) {
        case 1:
                callbacks.onMouseButton(Window_MouseButton_left, state);
                break;
        case 2:
                callbacks.onMouseButton(Window_MouseButton_middle, state);
                break;
        case 3:
                callbacks.onMouseButton(Window_MouseButton_right, state);
                break;
        case 4:
                callbacks.onMouseButton(Window_MouseButton_scrollUp, state);
                break;
        case 5:
                callbacks.onMouseButton(Window_MouseButton_scrollDown, state);
                break;
        }

        return Error_none;
}

/* Window_Stop
 * If the window is currently active, close it, and free the cairo rendering
 * context.
 */
Error Window_stop (void) {
        if (!started) { return Error_none; }

        listening = 0;
        started   = 0;

        cairo_surface_destroy(Window_surface);
        int status = XCloseDisplay(display);
        if (status != 1) { return Error_cantCloseDisplay; }

        return Error_none;
}

/* Window_setTitle
 * Sets the title that will be displayed by the window manager.
 */
Error Window_setTitle (const char *title) {
        int status = XStoreName(display, window, title);
        if (status != 1) { return Error_cantSetName; }

        return Error_none;
}

/* Window_onRedraw
 * Sets the function to be called when the window is redrawn. The new window
 * dimensions are passed as width and height.
 */
void Window_onRedraw (void (*callback) (int width, int height)) {
        callbacks.onRedraw = callback;
}

/* Window_onMouseButton
 * Sets the function to be called when a mouse button is pressed or released.
 * The mouse button that was pressed is passed as button, and whether it is
 * pressed or released is passed as state,
 */
void Window_onMouseButton (
        void (*callback) (
                Window_MouseButton button, Window_State state)
) {
        callbacks.onMouseButton = callback;
}

/* Window_onMouseMove
 * Sets the function to be called when the mouse is moved. The new mouse
 * position is passed as x and y.
 */
void Window_onMouseMove (void (*callback) (int x, int y)) {
        callbacks.onMouseMove = callback;
}
