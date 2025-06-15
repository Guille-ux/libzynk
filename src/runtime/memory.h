#ifndef ZYNK_MEMORY
#define ZYNK_MEMORY

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool zynk_cpy(uint8_t *dest, uint8_t *src, uint32_t len);
bool zynk_strcmp(const char *a, const char *b, uint32_t len);
uint32_t zynk_len(const char *str, char endChar);

#endif
