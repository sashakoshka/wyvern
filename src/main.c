#include <unistd.h>
#include <stdio.h>
#include "options.h"
#include "interface.h"
#include "edit-buffer.h"

int main () {
        Options_start();
        EditBuffer *editBuffer = EditBuffer_new();
        EditBuffer_open(editBuffer, "src/edit-buffer.c");
        Interface_setEditBuffer(editBuffer);
        Interface_run();
}
