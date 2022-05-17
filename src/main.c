#include <unistd.h>
#include <stdio.h>
#include "interface.h"
#include "edit-buffer.h"

int main () {
        // size_t amountGot;
        // Rune rune = Unicode_utf8ToRune("a", &amountGot);
        // printf("%i\n", rune);
        
        EditBuffer *editBuffer = EditBuffer_new();
        EditBuffer_open(editBuffer, "testfile2");
        Interface_setEditBuffer(editBuffer);
        Interface_run();
}
