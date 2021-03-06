#include <xkbcommon/xkbcommon.h>

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <cairo.h>
#include <cairo-xlib.h>

#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>

#include "window.h"

typedef unsigned long long Timestamp;

cairo_surface_t *Window_surface  = { 0 };
cairo_t         *Window_context  = { 0 };
time_t           Window_interval = 0;

static Timestamp previousTimestamp = 0;

static int width  = 640;
static int height = 480;
static int listening = 0;
static int started   = 0;

static Window   window  = { 0 };
static Display *display = { 0 };
static int      screen  = { 0 };

static Atom windowDeleteEvent;

static struct {
        void (*onRedraw)      (int, int, int);
        void (*onMouseButton) (int, Window_MouseButton, Window_State);
        void (*onMouseMove)   (int, int, int);
        void (*onInterval)    (int);
        void (*onKey)         (int, Window_KeySym, Rune, Window_State);
} callbacks = { 0 };

static Error respondToEvent       (int, XEvent);
static Error respondToEventButton (int, unsigned int, Window_State);
static Error respondToEventKey    (int, XKeyEvent *, Window_State);

static int       fileDescriptorTimeout (int, time_t);
static int       nextXEventOrTimeout   (XEvent *, time_t);
static Timestamp currentTimestamp      (void);

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
                int timedOut = nextXEventOrTimeout(&event, Window_interval);
                int render   = XEventsQueued(display, QueuedAfterFlush) < 2;

                cairo_push_group(Window_context);

                if (!timedOut) {
                        Error err = respondToEvent(render, event);
                        if (err) { return err; }
                }

                Timestamp newTimestamp = currentTimestamp();
                Timestamp maxTimestamp =
                        previousTimestamp +
                        (Timestamp)(Window_interval);
                if (timedOut || newTimestamp > maxTimestamp) {
                        previousTimestamp = newTimestamp;
                        if (callbacks.onInterval != NULL) {
                                callbacks.onInterval(1);
                        }
                }

                cairo_pop_group_to_source(Window_context);
                cairo_paint(Window_context);        
                cairo_surface_flush(Window_surface);
        }

        return Error_none;
}

/* respondToEvent
 * Handle a single event from the Xlib event loop in Window_listen.
 */
static Error respondToEvent (int render, XEvent event) {
        Error err;
        
        switch (event.type) {
        case ButtonPress:
                err = respondToEventButton (
                        render,
                        event.xbutton.button,
                        Window_State_on);
                if (err) { return err; }
                break;
        
        case ButtonRelease:
                err = respondToEventButton (
                        render,
                        event.xbutton.button,
                        Window_State_off);
                if (err) { return err; }
                break;

        case KeyPress:
                err = respondToEventKey(render, &event.xkey, Window_State_on);
                if (err) { return err; }
                break;
        
        case KeyRelease:
                err = respondToEventKey(render, &event.xkey, Window_State_off);
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
                        
                if (callbacks.onMouseMove == NULL) { break; }
                callbacks.onMouseMove(render, mouseX, mouseY);
                break;

        case Expose:
                if (callbacks.onRedraw == NULL) { break; }

                cairo_paint(Window_context);
                callbacks.onRedraw(render, width, height);
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
        int          render, 
        unsigned int button,
        Window_State state
) {
        if (callbacks.onMouseButton == NULL) { return Error_none; }
        
        switch (button) {
        case 1:
                callbacks.onMouseButton (
                        render,
                        Window_MouseButton_left,
                        state);
                break;
        case 2:
                callbacks.onMouseButton (
                        render,
                        Window_MouseButton_middle,
                        state);
                break;
        case 3:
                callbacks.onMouseButton (
                        render,
                        Window_MouseButton_right,
                        state);
                break;
        case 4:
                callbacks.onMouseButton (
                        render,
                        Window_MouseButton_scrollUp,
                        state);
                break;
        case 5:
                callbacks.onMouseButton (
                        render,
                        Window_MouseButton_scrollDown,
                        state);
                break;
        }

        return Error_none;
}

static Error respondToEventKey (
        int          render,
        XKeyEvent   *event,
        Window_State state
) {
        if (callbacks.onKey == NULL) { return Error_none; }
        
        KeySym keySym = XkbKeycodeToKeysym (
                display,
                (KeyCode)(event->keycode),
                0, event->state & ShiftMask ? 1 : 0);
        
        Rune rune = (Rune)(xkb_keysym_to_utf32((xkb_keysym_t)(keySym)));
        callbacks.onKey(render, keySym, rune, state);

        return Error_none;
}

/* fileDescriptorTimeout
 * Waits for an event on fileDescriptor, for the max amount of time specified by
 * milliseconds. Returns 1 if the timeout was reached.
 */
static int fileDescriptorTimeout (int fileDescriptor, time_t milliseconds) {
        fd_set fileDescriptorSet;
        FD_ZERO(&fileDescriptorSet);
        FD_SET(fileDescriptor, &fileDescriptorSet);
        
        struct timeval time = { 0 };
        time.tv_usec = milliseconds * 1000;
        
        return select(fileDescriptor + 1, &fileDescriptorSet, 0, 0, &time);
}

/* nextXEventOrTimeout
 * Gets the next X event, but only waits for the maximum amount of time
 * specified by milliseconds. If milliseconds is zero, there will be no timeout.
 * Returns 1 if the timeout was reached, and zero if an event was recieved.
 */
static int nextXEventOrTimeout (XEvent *event, time_t milliseconds) {
        int xFileDescriptor = ConnectionNumber(display);

        if (
                milliseconds == 0 ||
                XPending(display) ||
                fileDescriptorTimeout(xFileDescriptor, milliseconds)
        ) {
                XNextEvent(display, event);
                return 0;
        } else {
                return 1;
        }
}

/* currentTimestamp
 * Returns the current UNIX timestamp in milliseconds.
 */
static Timestamp currentTimestamp () {
        struct timeval time; 
        gettimeofday(&time, NULL);
        Timestamp milliseconds =
                (Timestamp) (time.tv_sec  * 1000) +
                (Timestamp) (time.tv_usec / 1000);
        return milliseconds;
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
void Window_onRedraw (void (*callback) (int render, int width, int height)) {
        callbacks.onRedraw = callback;
}

/* Window_onMouseButton
 * Sets the function to be called when a mouse button is pressed or released.
 * The mouse button that was pressed is passed as button, and whether it is
 * pressed or released is passed as state.
 */
void Window_onMouseButton (
        void (*callback) (
                int                render,
                Window_MouseButton button,
                Window_State       state)
) {
        callbacks.onMouseButton = callback;
}

/* Window_onMouseMove
 * Sets the function to be called when the mouse is moved. The new mouse
 * position is passed as x and y.
 */
void Window_onMouseMove (void (*callback) (int render, int x, int y)) {
        callbacks.onMouseMove = callback;
}

/* Window_onInterval
 * Sets the function to be called on an interval specified by Window_interval.
 */
void Window_onInterval (void (*callback) (int render)) {
        callbacks.onInterval = callback;
}

/* Window_onKey
 * Sets the function to be called when a key is pressed or released. The Xlib
 * keysym that was pressed is passed as keySym, and whether it is pressed or
 * released is passed as state. If the keySym has a utf32 representation, it is
 * passed as rune.
 */
void Window_onKey (
        void (*callback) (
                int           render,
                Window_KeySym keySym,
                Rune          rune,
                Window_State  state)
) {
        callbacks.onKey = callback;
}
