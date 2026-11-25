#ifndef KEEN1_UNLZEXE_H
#define KEEN1_UNLZEXE_H
#include <stddef.h>
#include <stdint.h>
/* Unpacks an LZEXE 0.91 packed DOS executable (KEEN1.EXE) into the raw load image (no MZ header).
   Returns a malloc'ed buffer (free() it) and its size in *size, or NULL on error (message in err, at least 128 bytes). */
uint8_t *unlzexe_file(const char *path, size_t *size, char *err);
#endif
