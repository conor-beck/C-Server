#include "utils.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "const.h"
#include <unistd.h>
//#include "extracredit.h"
void* threadTtl(void* vargp);
hashmap_t *create_map(uint32_t capacity, hash_func_f hash_function, destructor_f destroy_function) {
    if(capacity < 1){
        errno = EINVAL;
        return NULL;
    }
    if(hash_function == NULL){
        errno = EINVAL;
        return NULL;
    }
    if(destroy_function == NULL){
        errno = EINVAL;
        return NULL;
    }
    hashmap_t* hash = calloc(1, sizeof(hashmap_t));
    map_node_t* start = calloc(capacity, sizeof(map_node_t));
    if(hash == NULL){
        return NULL;
    }
    if(start == NULL){
        return NULL;
    }
    //map_node_t* ptr = start;

    //for(int i = 0; i < capacity; i++){
        //ptr = ptr + i;
        //ptr->tombstone = false;
    //}
    hash->capacity = capacity;
    hash->size = 0;
    hash->hash_function = hash_function;
    hash->destroy_function = destroy_function;
    hash->num_readers = 0;
    hash->nodes = start;
    hash->invalid = false;

    if(pthread_mutex_init(&hash->write_lock, NULL) != 0){
        //abort();
        return NULL;
    }
    if(pthread_mutex_init(&hash->fields_lock, NULL) != 0){
        //abort();
        return NULL;
    }
    return hash;
}

bool put(hashmap_t *self, map_key_t key, map_val_t val, bool force) {

    if(self == NULL){
        errno = EINVAL;
        return false;
    }
    if(self->invalid == true){
        errno = EINVAL;
        return false;
    }
    if(key.key_len < 1){
        errno = EINVAL;
        return false;
    }
    if(key.key_base == NULL){
        errno = EINVAL;
        return false;
    }
    if(val.val_len < 1){
        errno = EINVAL;
        return false;
    }
    if(val.val_base == NULL){
        errno = EINVAL;
        return false;
    }
    if(self->size == self->capacity && force == false){
        errno = ENOMEM;
        return false;
    }

    //ptr = ptr + index;

    int found = 0;
    if(pthread_mutex_lock(&self->write_lock) != 0){
        abort();
    }
    int index = get_index(self, key);
    int startIndex = index;
    map_node_t* ptr = self->nodes;
    map_node_t* fullPtr = self->nodes;
    int biggest = -5;
    int biggestIndex = -5;
    //spot = index;
    ptr = ptr + index;
    if(self->size == self->capacity && force == true){
        int i = 0;
        while(i != self->capacity){
            fullPtr = fullPtr + i;
            if(fullPtr->key.key_len != 0){
                if(fullPtr->lastUsed > biggest){
                    biggest = fullPtr->lastUsed;
                    biggestIndex = i;
                }
            }
            i = i + 1;
            fullPtr = self->nodes;
        }
        //if(ptr->tombstone == true){
        if(biggest == -5){
            self->destroy_function(ptr->key, ptr->val);
            ptr->key.key_len = key.key_len;
            ptr->key.key_base = key.key_base;
            ptr->val.val_len = val.val_len;
            ptr->val.val_base = val.val_base;
            ptr->lastUsed = 0;
            //ptr->id = iden;
            //self->size = self->size + 1;
            found = 1;
        }
        else{
            fullPtr = self->nodes;
            fullPtr = fullPtr + biggestIndex;
            self->destroy_function(fullPtr->key, fullPtr->val);
            fullPtr->key.key_len = key.key_len;
            fullPtr->key.key_base = key.key_base;
            fullPtr->val.val_len = val.val_len;
            fullPtr->val.val_base = val.val_base;
            fullPtr->lastUsed = 0;
            //ptr->id = iden;
            //self->size = self->size + 1;
            found = 1;
        }
    }
    else{
        found = 0;
        //int spot = 1;
        while(found == 0){

            if(ptr->tombstone == true){
                found = 1;
                ptr->tombstone = false;
                ptr->key.key_len = key.key_len;
                ptr->key.key_base = key.key_base;
                ptr->val.val_len = val.val_len;
                ptr->val.val_base = val.val_base;
                self->size = self->size  + 1;
            }
            else if(ptr->key.key_len == 0){
                found = 1;
                ptr->key.key_len = key.key_len;
                ptr->key.key_base = key.key_base;
                ptr->val.val_len = val.val_len;
                ptr->val.val_base = val.val_base;
                ptr->lastUsed = 0;
                self->size = self->size  + 1;
            }
            else if(ptr->key.key_len == key.key_len){
                if(memcmp(ptr->key.key_base, key.key_base, key.key_len) == 0){
                    found = 1;
                    ptr->val.val_len = val.val_len;
                    ptr->val.val_base = val.val_base;
                    ptr->lastUsed = 0;
                }
                else{
                    if(index == (self->capacity - 1)){
                        ptr = self->nodes;
                        index = 0;
                        if(index == startIndex){
                            break;
                        }

                    }
                    else{
                        ptr = ptr + 1;
                        index = index + 1;
                        if(index == startIndex){
                            break;
                        }
                    }
                }
            }
            else{
                if(index == (self->capacity - 1)){
                    ptr = self->nodes;
                    index = 0;
                    if(index == startIndex){
                        break;
                    }

                }
                else{
                    ptr = ptr + 1;
                    index = index + 1;
                    if(index == startIndex){
                        break;
                    }
                }
            }
        }
    }

    if(pthread_mutex_unlock(&self->write_lock) != 0){
        abort();
    }


    if(found == 1){
        return true;
    }
    else{
        return false;
    }
}

map_val_t get(hashmap_t *self, map_key_t key) {
    int spot = 1;
    size_t len;
    void* base;
    int found = 0;
    if(self == NULL){
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }
    if(self->invalid == true){
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }
    if(key.key_len < 1){
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }
    if(key.key_base == NULL){
        errno = EINVAL;
        return MAP_VAL(NULL, 0);
    }
    if(self->size == 0){
        return MAP_VAL(NULL, 0);
    }
    if(pthread_mutex_lock(&self->fields_lock) != 0){
        abort();
    }
    self->num_readers = self->num_readers + 1;
    if(pthread_mutex_lock(&self->write_lock) != 0){
        abort();
    }
    if(pthread_mutex_unlock(&self->fields_lock) != 0){
        abort();
    }
    map_node_t* ptr = self->nodes;
    int index = get_index(self, key);
    spot = index;
    ptr = ptr + index;
    int count = 0;
    while(found == 0){
        if(ptr->tombstone == false){
            if(ptr->key.key_len == 0){
                break;
            }
            else{
                if(ptr->key.key_len == key.key_len){
                    if(memcmp(ptr->key.key_base, key.key_base, key.key_len) == 0){
                        len = ptr->val.val_len;
                        base = ptr->val.val_base;
                        found = 1;
                    }
                    else{
                        if(spot == (self->capacity - 1)){
                            ptr = self->nodes;
                            spot = 0;
                            count = count + 1;
                            if(count == self->capacity){
                                break;
                            }
                        }
                        else{
                            spot = spot + 1;
                            ptr = ptr + 1;
                            count = count + 1;
                            if(count == self->capacity){
                                break;
                            }
                        }
                    }
                }
                else{
                    if(spot == (self->capacity - 1)){
                        ptr = self->nodes;
                        spot = 0;
                        count = count + 1;
                        if(count == self->capacity){
                            break;
                        }
                    }
                    else{
                        spot = spot + 1;
                        ptr = ptr + 1;
                        count = count + 1;
                        if(count == self->capacity){
                            break;
                        }
                    }
                }
            }
        }
        //a
        else{
            if(spot == (self->capacity - 1)){
                ptr = self->nodes;
                spot = 0;
                count = count + 1;
                if(count == self->capacity){
                    break;
                }
            }
            else{
                spot = spot + 1;
                ptr = ptr + 1;
                count = count + 1;
                if(count == self->capacity){
                    break;
                }
            }
        }
    }

    int i = 0;
    map_node_t* ecPtr = self->nodes;
    while(i < self->capacity){
        ecPtr = ecPtr + i;
        if(i != spot && ecPtr->key.key_len != 0){
            ecPtr->lastUsed = ecPtr->lastUsed + 1;
        }


        i = i + 1;
        ecPtr = self->nodes;
    }
    if(found == 1){
        if(pthread_mutex_lock(&self->fields_lock) != 0){
            abort();
        }
        self->num_readers = self->num_readers - 1;
        if(self->num_readers == 0){
            if(pthread_mutex_unlock(&self->write_lock) != 0){
                abort();
            }
        }
        if(pthread_mutex_unlock(&self->fields_lock) != 0){
            abort();
        }
        return MAP_VAL(base, len);
    }
    else{
        if(pthread_mutex_lock(&self->fields_lock) != 0){
            abort();
        }
        self->num_readers = self->num_readers - 1;
        if(self->num_readers == 0){
            if(pthread_mutex_unlock(&self->write_lock) != 0){
                abort();
            }
        }
        if(pthread_mutex_unlock(&self->fields_lock) != 0){
            abort();
        }
        return MAP_VAL(NULL, 0);
    }
}

map_node_t delete(hashmap_t *self, map_key_t key) {
    int spot = 1;
    size_t len;
    void* base;
    size_t keyLen;
    void* keyBase;
    int found = 0;
    map_node_t nRet;
    nRet.key.key_len = 0;
    nRet.key.key_base = NULL;
    nRet.val.val_base = NULL;
    nRet.val.val_len = 0;
    if(self == NULL){
        errno = EINVAL;
        return nRet;
    }
    if(self->invalid == true){
        errno = EINVAL;
        return nRet;
    }
    if(key.key_len < 1){
        errno = EINVAL;
        return nRet;
    }
    if(key.key_base == NULL){
        errno = EINVAL;
        return nRet;
    }
    if(self->size == 0){
        return nRet;
    }

    if(pthread_mutex_lock(&self->write_lock) != 0){
        abort();
    }

    map_node_t* ptr = self->nodes;
    spot = get_index(self, key);
    ptr = ptr + spot;
    int count = 0;
    while(found == 0){
        if(ptr->tombstone == false){
            if(ptr->key.key_len == key.key_len){
                if(memcmp(ptr->key.key_base, key.key_base, key.key_len) == 0){
                    len = ptr->val.val_len;
                    base = ptr->val.val_base;
                    keyLen = ptr->key.key_len;
                    keyBase = ptr->key.key_base;
                    ptr->tombstone = true;
                    self->size = self->size - 1;
                    //self->destroy_function(ptr->key, ptr->val);
                    found = 1;
                }
                else{
                    if(spot == (self->capacity - 1)){
                        ptr = self->nodes;
                        spot = 0;
                        count = count + 1;
                        if(count == self->capacity){
                            break;
                        }
                    }
                    else{
                        spot = spot + 1;
                        ptr = ptr + 1;
                        count = count + 1;
                        if(count == self->capacity){
                            break;
                        }
                    }
                }
            }
            else{
                if(spot == (self->capacity - 1)){
                    ptr = self->nodes;
                    spot = 0;
                    count = count + 1;
                    if(count == self->capacity){
                        break;
                    }
                }
                else{
                    spot = spot + 1;
                    ptr = ptr + 1;
                    count = count + 1;
                    if(count == self->capacity){
                        break;
                    }
                }
            }
        }
        else{
            if(spot == (self->capacity - 1)){
                ptr = self->nodes;
                spot = 0;
                count = count + 1;
                if(count == self->capacity){
                    break;
                }
            }
            else{
                spot = spot + 1;
                ptr = ptr + 1;
                count = count + 1;
                if(count == self->capacity){
                    break;
                }
            }
        }
    }
    if(found == 1){
        if(pthread_mutex_unlock(&self->write_lock) != 0){
            abort();
        }
        return MAP_NODE(MAP_KEY(keyBase, keyLen), MAP_VAL(base, len), true);
    }
    else{
        if(pthread_mutex_unlock(&self->write_lock) != 0){
            abort();
        }
        return MAP_NODE(MAP_KEY(NULL, 0), MAP_VAL(NULL, 0), false);
    }
}

bool clear_map(hashmap_t *self) {
	if(self == NULL){
        errno = EINVAL;
        return false;
    }
    if(self->invalid == true){
        errno = EINVAL;
        return false;
    }
    if(self->size < 1){
        //errno = EINVAL;
        return true;
    }
    int count = 0;
    if(pthread_mutex_lock(&self->write_lock) != 0){
        abort();
    }
    map_node_t* ptr = self->nodes;
    while(count != self->size){
        if(ptr->key.key_len != 0){
            count = count + 1;
            self->destroy_function(ptr->key, ptr->val);
            ptr->key.key_len = 0;
            ptr->val.val_len = 0;
            ptr->tombstone = false;
            ptr = ptr + 1;
            //printf("yo\n");
        }
        else{
            ptr = ptr + 1;
        }
    }
    ptr = self->nodes;
    int i = 0;
    while(i < self->capacity){
        ptr = ptr + i;
        ptr->tombstone = false;

        i = i + 1;
        ptr = self->nodes;
    }
    self->size = 0;
    //free(self->nodes);
    //map_node_t* newNodes = calloc(self->capacity, sizeof(map_node_t));
    //self->nodes = newNodes;
    if(pthread_mutex_unlock(&self->write_lock) != 0){
        abort();
    }
    return true;
}

bool invalidate_map(hashmap_t *self) {
    if(self == NULL){
        errno = EINVAL;
        return false;
    }
    int count = 0;
    if(pthread_mutex_lock(&self->write_lock) != 0){
        abort();
    }
    map_node_t* ptr = self->nodes;
    while(count != self->size){
        if(ptr->key.key_len != 0){
            count = count + 1;
            self->destroy_function(ptr->key, ptr->val);
            ptr = ptr + 1;
        }
        else{
            ptr = ptr + 1;
        }
    }
    free(self->nodes);
    self->invalid = true;
    if(pthread_mutex_unlock(&self->write_lock) != 0){
        abort();
    }
    return true;
}

