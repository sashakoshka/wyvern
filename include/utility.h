#include <stddef.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

size_t Utility_constrainChange (size_t, int, size_t);
size_t Utility_addToSizeT      (size_t, int);
char  *Utility_copyCString     (char *, const char *, size_t);
