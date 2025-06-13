#ifndef ZYNK_ENVIROMENT
#define ZYNK_ENVIROMENT

#include "../common.h"
#include "types.h"

typedef struct ZynkEnvEntry {
  char *name;
  Value value;
  struct ZynkEnvEntry *next; // manejo de colisiones
} ZynkEnvEntry;

typedef struct ZynkEnvTable {
  ZynkEnvEntry** entries;
  size_t capacity;
  size_t count;
} ZynkEnvTable;

typedef struct ZynkEnv {
  ZynkEnvTable local;
  ZynkEnv *enclosing;
} ZynkEnv;



#endif
