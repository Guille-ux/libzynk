#ifndef ZYNKL_VALUE
#define ZYNKL_VALUE

#include "../common.h"
#include "objects.h"

typedef enum {
  ZYNK_NULL,
  ZYNK_BOOL,
  ZYNK_NUMBER,
  ZYNK_OBJ,
  ZYNK_BYTE,
} ZYNK_TYPE;

typedef enum {
  ObjString,
  ObjFunction,
  ObjArray,
} ObjType;

struct ZynkObj;

typedef struct {
  ZYNK_TYPE type;
  union {
    bool boolean;
    double number;
    uint8_t byte;
    ZynkObj *obj;
  } as;
  uint32_t ref_count; // para el conteo de referencias
} Value;

struct ZynkObj {
  ObjType type;
  union {
    struct ZynkString *string;
    struct ZynkFunction *function;
    struct ZynkArray *array;
  } obj;
};

typedef struct ZynkObj ZynkObj;

#endif
