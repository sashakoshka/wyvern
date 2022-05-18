#include "options.h"

int Options_tabSize;
int Options_tabsToSpaces;

/* Options_start
 * Intializes the options module.
 */
void Options_start () {
        Options_tabSize      = 4;
        Options_tabsToSpaces = 1;
}
