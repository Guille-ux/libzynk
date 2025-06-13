#ifndef LIBZYNK_COMMON
#define LIBZYNK_COMMON

#include <stdint.h>
#include <stddef.h>

#include "../sysarena/include/sysarena.h"
#include "../sysarena/include/types.h"

#include "runtime/objects.h"
#include "runtime/types.h"
#include "runtime/zynk_enviroment.h"

typedef Value (*ZynkFuncPtr)(ZynkEnv* env, struct ZynkArray* args);

#endif
