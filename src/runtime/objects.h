#ifndef ZYNK_OBJECTS
#define ZYNK_OBJECTS

#include "../common.h"
#include "types.h"     



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


#endif // ZYNKL_OBJECTS
