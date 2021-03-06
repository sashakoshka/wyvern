#include "utility.h"

/* Utility_constrainChange
 * Constrains a change in initial of amount, to a lower bound of 0 and an upper
 * bound of bound (bound is not inclusive).
 */
size_t Utility_constrainChange (size_t initial, int amount, size_t bound) {
        size_t amountAbs;
        
        if (amount < 0) {
                amountAbs = (size_t)(0 - amount);
                if (initial >= amountAbs) {
                        initial -= amountAbs;
                } else {
                        initial = 0;
                }
        } else {
                amountAbs = (size_t)amount;
                initial += amountAbs;
        }
        
        if (initial >= bound) {
                initial = bound - 1;
        }

        return initial;
}

/* Utility_addToSizeT
 * Behaves the same as constrainChange, but does not specify an upper bound.
 */
size_t Utility_addToSizeT (size_t initial, int amount) {
        size_t amountAbs;
        
        if (amount < 0) {
                amountAbs = (size_t)(0 - amount);
                return initial - amountAbs;
        } else {
                amountAbs = (size_t)(amount);
                return initial + amountAbs;
        }
}

/* Utility_copyCString
 * Secure alternative to strncpy. Returns a pointer to destination.
 */
char *Utility_copyCString (
        char       *destination,
        const char *source,
        size_t      maximumChars
) {
        if (source == NULL) {
                destination[0] = '\0';
                return destination;
        }
        
        size_t index;
        for (index = 0; index < maximumChars; index ++) {
                if (source[index] == '\0') { break; }
                destination[index] = source[index];
        }

        destination[index] = '\0';
        
        return destination;
}
