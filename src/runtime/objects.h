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
};


struct ZynkNativeFunction {
  const char *name;
  ZynkFuncPtr func_ptr; 
};

struct ZynkArray {
  size_t len;
  size_t capacity;
  Value* array; 
};

Value zynkArrayGet(Value array_val, Value index_val);
void zynkArraySet(ArenaManager *manager, Value array_val, Value index_val, Value new_element);
Value zynkArrayPop(ArenaManager *manager, Value array_val);

#endif // ZYNKL_OBJECTS
