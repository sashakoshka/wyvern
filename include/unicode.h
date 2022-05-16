#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef uint32_t Rune;

#define UTF8_1B_MASK        0b10000000
#define UTF8_1B_CHECK       0b00000000
#define UTF8_1B_INVERT_MASK ~UTF8_1B_MASK
#define UTF8_2B_MASK        0b11100000
#define UTF8_2B_CHECK       0b11000000
#define UTF8_2B_INVERT_MASK ~UTF8_2B_MASK
#define UTF8_3B_MASK        0b11110000
#define UTF8_3B_CHECK       0b11100000
#define UTF8_3B_INVERT_MASK ~UTF8_3B_MASK
#define UTF8_4B_MASK        0b11111000
#define UTF8_4B_CHECK       0b11110000
#define UTF8_4B_INVERT_MASK ~UTF8_4B_MASK

#define UTF8_CONTINUE_MASK        0b11000000
#define UTF8_CONTINUE_INVERT_MASK ~UTF8_CONTINUE_MASK

size_t Unicode_utf8CodepointSize (uint8_t ch);
Rune   Unicode_utf8ToRune        (const char *, size_t *);
Rune   Unicode_utf8FileGetRune   (FILE *, int *); 
Rune   Unicode_utf8ArrayToRune   (const uint8_t[4], size_t);
