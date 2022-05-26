#include "options.h"

int    Options_tabSize;
int    Options_tabsToSpaces;
size_t Options_columnGuide;
int    Options_scrollSize;
int    Options_cursorSize;
int    Options_fontSize;
char  *Options_fontName;

/* Options_start
 * Intializes the options module.
 */
void Options_start () {
        Options_tabSize      = 8;
        Options_tabsToSpaces = 1;
        Options_columnGuide  = 80;
        Options_scrollSize   = 8;
        Options_cursorSize   = 2;
        Options_fontSize     = 14;
        Options_fontName     =
                "/home/sashakoshka/.local/share/fonts/DMMono-Light.ttf";
}
