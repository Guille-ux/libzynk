#include "object_mng.h"

static ZynkObj* create_base_zynk_obj(ArenaManager* manager, ObjType type) {
    if (manager == NULL) return NULL;

    ZynkObj* obj = (ZynkObj*)sysarena_alloc(manager, sizeof(ZynkObj));
    if (obj == NULL) return NULL; // Error

    obj->type = type;
    obj->ref_count = 1;
    return obj;
}

Value zynkCreateString(ArenaManager *manager, const char *str) {
  if (manager==NULL || str==NULL) return zynkNull();
  
  size_t strlen = zynk_len(str, END_CHAR);
  
  ZynkObj *obj = create_base_zynk_obj(manager, ObjString);

  if (obj==NULL) return zynkNull();

  ZynkString *string=(ZynkString *)sysarena_alloc(manager, sizeof(ZynkString));
  if (string==NULL) {
    sysarena_free(manager, (void *)obj);
    return zynkNull();
  }

  string->string=(char *)sysarena_alloc(manager, strlen+1);
  if (string->string==NULL) {
    sysarena_free(manager, (void*)string);
    sysarena_free(manager, (void*)obj);
    return zynkNull();
  }
  zynk_cpy((uint8_t*)string->string, (uint8_t*)str, strlen);
  string->string[strlen]='\0';
  string->len=strlen;

  obj->obj.string=string;

  Value ret;
  ret.type=ZYNK_OBJ;
  ret.as.obj=obj;
  return ret;
}
