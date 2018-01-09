#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>

queue_t *create_queue(void) {
    queue_t* qu = calloc(1, sizeof(queue_t));
    if(qu == NULL){
        //abort();
        return NULL;
    }
    qu->front = NULL;
    qu->rear = NULL;
    if(sem_init(&qu->items, 0, 0) != 0){
        return NULL;
    }
    qu->invalid = false;
    if(pthread_mutex_init(&qu->lock, NULL) != 0){
        //bort();
        return NULL;
    }
    return qu;
}

bool invalidate_queue(queue_t *self, item_destructor_f destroy_function) {
    if(self == NULL){
        errno = EINVAL;
        return false;
    }
    int size;
    queue_node_t* check;
    if(pthread_mutex_lock(&self->lock) != 0){
        abort();
    }
    if(sem_getvalue(&self->items, &size) != 0){
        abort();
    }
    check = self->front;
    if(check == NULL){
        self->invalid = true;
    }
    else{
        while(check != NULL){
            check = self->front->next;
            destroy_function(self->front->item);
            free(self->front);
            self->front = check;
        }
    }
    self->invalid = true;
    if(pthread_mutex_unlock(&self->lock) != 0){
        abort();
    }
    return true;
}

bool enqueue(queue_t *self, void *item) {
    int size;
    if(self == NULL || item == NULL){
        errno = EINVAL;
        return false;
    }
    queue_node_t* node = calloc(1, sizeof(queue_node_t));
    if(node == NULL){
        return false;
    }
    node->item = item;
    node->next = NULL;
    if(pthread_mutex_lock(&self->lock) != 0 ){
        abort();
    }
    if(sem_getvalue(&self->items, &size) != 0){
        abort();
    }
    if(size == 0){
        self->front = node;
        self->rear = node;
    }
    else{
        self->rear->next = node;
        self->rear = node;
    }
    if(sem_post(&self->items) != 0){
        abort();
    }
    if(pthread_mutex_unlock(&self->lock) != 0){
        abort();
    }
    return true;
}

void *dequeue(queue_t *self) {
    if(self == NULL){
        errno = EINVAL;
        return NULL;
    }
    int size;
    if(sem_wait(&self->items) != 0){
        abort();
    }
    if(pthread_mutex_lock(&self->lock) != 0){
        abort();
    }
    if(sem_getvalue(&self->items, &size) != 0){
        abort();
    }
    void* ret = self->front->item;
    if(size == 0){
        free(self->front);
        self->front = NULL;
        self->rear = NULL;
    }
    else{
        queue_node_t* temp = self->front->next;
        free(self->front);
        self->front = temp;
    }
    if(pthread_mutex_unlock(&self->lock) != 0){
        abort();
    }
    return ret;
}
