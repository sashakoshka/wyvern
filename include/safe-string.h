#pragma once

#include <stdlib.h>
#include <stdint.h>
#include "unicode.h"

typedef struct {
        size_t size;
        size_t length;
        Rune  *buffer;
} String;

String *String_new   (const char *);
void    String_free  (String *);
void    String_clear (String *);

void String_addBuffer (String *, const char *);
void String_addString (String *, String *);
void String_addRune   (String *, Rune);

void String_insertBuffer (String *, const char *, size_t);
void String_insertString (String *, String *, size_t);
void String_insertRune   (String *, Rune, size_t);

void String_deleteRune  (String *, size_t);
void String_deleteRange (String *, size_t, size_t);
