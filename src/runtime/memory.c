// zynk memory implementation
#include "memory.h"

bool zynk_cpy(uint8_t *dest, uint8_t *src, uint32_t len) {
  if (dest==NULL || src==NULL) {
    return false;
  }
  for (uint32_t i=0;i<len;i++) {
    dest[i]=src[i];
  }
  return true;
}

bool zynk_strcmp(const char *a, const char *b, uint32_t len) {
  for (uint32_t i=0;i<len;i++) {
    if (a[i]!=b[i]) {
      return false;
    }
  }
  return true;
}

uint32_t zynk_len(const char *str, char endChar) {
  if (str==NULL) {
    return 0;
  }
  for (uint32_t i=0;;i++) {
    if (str[i]==endChar) {
      return i;
    }
  }
}
