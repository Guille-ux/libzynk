#ifndef ZYNKL_OBJECTS
#define ZYNKL_OBJECTS

#include "../common.h"

struct ZynkString {
  char *string;
  size_t len;
};

struct ZynkFunction {
  const char *name;
  ZynkFuncPtr func_ptr;
};

struct ZynkArray {
  size_t len;
  Value* array;
};

#endif
