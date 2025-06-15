#ifndef ZYNK_VALUE
#define ZYNK_VALUE


#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


struct ZynkObj;
struct Value;
struct ZynkString;
struct ZynkFunction;
struct ZynkArray;
struct ZynkEnv;
struct ZynkEnvTable;
struct ZynkEnvEntry;


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



typedef struct ZynkObj ZynkObj;
typedef struct Value Value;
typedef struct ZynkString ZynkString;
typedef struct ZynkFunction ZynkFunction;
typedef struct ZynkArray ZynkArray;
typedef struct ZynkEnv ZynkEnv;
typedef struct ZynkEnvTable ZynkEnvTable;
typedef struct ZynkEnvEntry ZynkEnvEntry;



typedef Value (*ZynkFuncPtr)(ZynkEnv* env, ZynkArray* args);



struct Value {
  ZYNK_TYPE type;
  union {
    bool boolean;
    double number;
    uint8_t byte;
    ZynkObj *obj; 
  } as;
  uint32_t ref_count; 
};

struct ZynkObj {
  ObjType type;
  union {
    ZynkString *string;   
    ZynkFunction *function; 
    ZynkArray *array;    
  } obj;
};

#endif // ZYNK_VALUE
