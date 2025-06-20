#ifndef ZYNK_CALLS
#define ZYNK_CALLS

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../common.h"
#include "types.h"
#include "zynk_enviroment.h"
#include "objects.h"
#include "object_mng.h"
#include "object_rf.h"

Value zynkCallFunction(ZynkEnv *env, const char *name, ZynkArray args);

#endif
