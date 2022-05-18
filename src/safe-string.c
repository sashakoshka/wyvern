#include <string.h>
#include <stdio.h>
#include "safe-string.h"

static void String_realloc (String *, size_t);

/* String_new
 * Creates a new string from the specified buffer.
 */
String *String_new (const char *buffer) {
        String *string = calloc(1, sizeof(String));
        string->length = 0;
        string->size   = string->length + 1;
        string->buffer = calloc(string->size, sizeof(Rune));

        String_addBuffer(string, buffer);
        return string;
}

/* String_free
 * Frees a string from memory.
 */
void String_free (String *string) {
        free(string->buffer);
        free(string);
}

/* String_clear
 * Resets a string, clearing all text inside of it.
 */
void String_clear (String *string) {
        string->length = 0;
        string->size   = string->length + 1;
        string->buffer = realloc(string->buffer, string->size);
}

/* String_addBuffer
 * Appends a buffer of chars to the end of a string.
 */
void String_addBuffer (String *string, const char *buffer) {
        for (int index = 0; buffer[index];) {
                size_t amountGot = 0;
                Rune rune = Unicode_utf8ToRune(buffer + index, &amountGot);
                index    += amountGot;
                
                String_addRune(string, rune);
        }
}

/* String_addString
 * Appends another string to the end of a string.
 */
void String_addString (String *string, String *addition) {
        size_t previousEnd = string->length;
        String_realloc(string, string->length + addition->length);

        for (size_t index = 0; index < addition->length; index ++) {
                string->buffer[previousEnd + index] = addition->buffer[index];
        }
}

/* String_addRune
 * Appends a single rune to the end of a string.
 */
void String_addRune (String *string, Rune rune) {
        string->buffer[string->length] = rune;
        String_realloc(string, string->length + 1);
}

/* String_insertBuffer
 * Inserts a buffer of chars into a particular point in a string. If position is
 * larger than the string length, this function does nothing.
 */
// TODO: Fix and re-comission
// void String_insertBuffer (
        // String     *string,
        // const char *buffer,
        // size_t     position
// ) {
        // if (position > string->length) { return; }
// 
        // size_t bufferLength = strlen(buffer);
        // String_realloc(string, string->length + bufferLength);
        // 
        // for (size_t index = 0; index < bufferLength; index ++) {
                // string->buffer[position + index + bufferLength] =
                        // string->buffer[position + index];
                // string->buffer[position + index] = buffer[index];
        // }
// }

/* String_insertString
 * Inserts another string into a particular point in a string. If position is
 * larger than the string length, this function does nothing.
 */
void String_insertString (
        String *string,
        String *addition,
        size_t position
) {
        if (position > string->length) { return; }
        
        String_realloc(string, string->length + addition->length);
        
        for (size_t index = 0; index < addition->length; index ++) {
                string->buffer[position + index + addition->length] =
                        string->buffer[position + index];
                string->buffer[position + index] = addition->buffer[index];
        }
}

/* String_insertRune
 * Inserts a single rune into a particular point in a string. If position is
 * larger than the string length, this function does nothing.
 */
void String_insertRune (String *string, Rune rune, size_t position) {
        if (position > string->length) { return; }
        
        String_realloc(string, string->length + 1);

        Rune new = rune;
        for (size_t index = position; index < string->size; index ++) {
                Rune previous = string->buffer[index];
                string->buffer[index] = new;
                new = previous;
        }
}

/* String_deleteRune
 * Deletes the rune from the string located at position. If position is larger
 * than or equal to the string length, this function does nothing.
 */
void String_deleteRune (String *string, size_t position) {
        String_deleteRange(string, position, position);
}

/* String_deleteRange
 * Delete from start to end, inclusive. If the bigger value is larger than or
 * equal to the string length, this function does nothing.
 */
void String_deleteRange (String *string, size_t start, size_t end) {
        if (start > end) {
                size_t swap = start;
                start = end;
                end = swap;
        }
        if (end >= string->length) { return; }

        end += 1;
        size_t offset = end - start;
        for (size_t index = end; index < string->length; index ++) {
                string->buffer[index - offset] = string->buffer[index];
        }

        String_realloc(string, string->length - offset);
}

/* String_splitInto
 * Removes all characters after point (inclusive), and adds them to destination.
 */
void String_splitInto (String *string, String *destination, size_t point) {
        for (size_t index = point; index < string->length; index ++) {
                String_addRune(destination, string->buffer[index]);
        }

        String_realloc(string, point);
}

/* String_realloc
 * Resizes the internal buffer of the string to accomodate a string of
 * newLength, excluding the null terminator.
 */
static void String_realloc (String *string, size_t newLength) {
        if (newLength == string->length) { return; }

        if (newLength < string->size) {
                // if the string is shrinking, just set the size to the new
                // size.
                string->size = newLength;
        } else {
                // try multiplying the current size by 2
                string->size *= 2;
                // if that isn't enough, just exactly match the new size.
                if (newLength > string->size) { string->size = newLength; }
        }
        
        string->buffer = realloc(string->buffer, string->size * sizeof(Rune));
        string->length = newLength;
}
