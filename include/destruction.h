#ifndef DESTRUCTION_H
#define DESTRUCTION_H
#include <stdlib.h>
#include <stdio.h>
//#include "hashmap.h"
#include "utils.h"
#include "queue.h"

void queueDestroy(void* item);
void mapDestroy(map_key_t key, map_val_t val);
#endif