#include "destruction.h"
#include <stdio.h>
#include <stdlib.h>

void queueDestroy(void *item) {
    free(item);
}

void mapDestroy(map_key_t key, map_val_t val){
    free(key.key_base);
    free(val.val_base);
}