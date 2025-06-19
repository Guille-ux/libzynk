#ifndef ZYNK_OBJECT_MANAGER
#define ZYNK_OBJECT_MANAGER

#include "../zynk.h"
#include "../common.h"
#include "types.h"
#include "objects.h"
#include "realloc.h"

Value zynkCreateNativeFunction(ArenaManager *manager, const char *name, ZynkFuncPtr func_ptr);
Value zynkCreateString(ArenaManager *manager, const char *str);
Value zynkCreateArray(ArenaManager *manager, size_t initial_capacity);

#endif
