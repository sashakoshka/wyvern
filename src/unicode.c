#include "unicode.h"

/* Unicode_utf8ToRune
 * Takes in a pointer to a char in the middle of a c string, and converts it,
 * possibly along with characters that came after it, into a UTF-32 rune.
 * The amount of bytes read in total (usually 1) is stored in amountGot.
 */
Rune Unicode_utf8ToRune (const char *buffer, size_t *amountGot) {
        size_t codepointSize = Unicode_utf8CodepointSize((uint8_t)buffer[0]);
        *amountGot = 0;
        
        size_t codepointSizeLimit = 4;
        uint8_t parts[4] = { 0 };

        for (size_t index = 0; index < codepointSize; index ++) {
                if (!buffer[index]) {
                        codepointSizeLimit = index;
                        break;
                }

                parts[index] = (uint8_t)buffer[index];
        }
        
        if (codepointSize > codepointSizeLimit) { return 0; }
        *amountGot = codepointSize;

        return Unicode_utf8ArrayToRune(parts, codepointSize);
}

/* Unicode_utf8FileGetRune
 * Takes in a file pointer, and gets and returns a single rune from it. If the
 * function encounters an EOF, reachedEnd will be set to 1.
 */
Rune Unicode_utf8FileGetRune (FILE *file, int *reachedEnd) {
        size_t codepointSize = 1;
        
        size_t codepointSizeLimit = 4;
        uint8_t parts[4] = { 0 };

        for (size_t index = 0; index < codepointSize; index ++) {
                int ch = fgetc(file);
                if (ch == EOF) {
                        *reachedEnd = 1;
                        codepointSizeLimit = index;
                        break;
                }

                if (index == 0) {
                        codepointSize =
                                Unicode_utf8CodepointSize((uint8_t)(ch));
                }

                parts[index] = (uint8_t)(ch);
        }
        
        if (codepointSize > codepointSizeLimit) { return 0; }
        return Unicode_utf8ArrayToRune(parts, codepointSize);
}

/* Unicode_utf8ArrayToRune
 * Takes in an array of four bytes, a codepoint size, and returns the
 * corresponding rune.
 */
Rune Unicode_utf8ArrayToRune (const uint8_t parts[4], size_t codepointSize) {
        Rune rune = 0;
        switch (codepointSize) {
        case 1:
                rune = (Rune)(parts[0] & UTF8_1B_INVERT_MASK);
                break;
        case 2:
                rune  = (Rune)(parts[0] & UTF8_2B_INVERT_MASK      ) << 6;
                rune |= (Rune)(parts[1] & UTF8_CONTINUE_INVERT_MASK);
                break;
        case 3:
                rune  = (Rune)(parts[0] & UTF8_3B_INVERT_MASK      ) << 12;
                rune |= (Rune)(parts[1] & UTF8_CONTINUE_INVERT_MASK) << 6;
                rune |= (Rune)(parts[2] & UTF8_CONTINUE_INVERT_MASK);
                break;
        case 4:
                rune  = (Rune)(parts[0] & UTF8_4B_INVERT_MASK      ) << 18;
                rune |= (Rune)(parts[1] & UTF8_CONTINUE_INVERT_MASK) << 12;
                rune |= (Rune)(parts[2] & UTF8_CONTINUE_INVERT_MASK) << 6;
                rune |= (Rune)(parts[3] & UTF8_CONTINUE_INVERT_MASK);
                break;
        }
        return rune;
}

/* Unicode_utf8CodepointSize
 * Returns the codepoint size of a single UTF-8 byte.
 */
size_t Unicode_utf8CodepointSize (uint8_t ch) {
        if ((ch & UTF8_1B_MASK) == UTF8_1B_CHECK) { return 1; }
        if ((ch & UTF8_2B_MASK) == UTF8_2B_CHECK) { return 2; }
        if ((ch & UTF8_3B_MASK) == UTF8_3B_CHECK) { return 3; }
        if ((ch & UTF8_4B_MASK) == UTF8_4B_CHECK) { return 4; }

        return 0;
}
