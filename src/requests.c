#include "requests.h"
#include <stdlib.h>
#include <stdio.h>
#include <helpers.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "utils.h"


int putRequest(request_header_t* header, int confd, hashmap_t* theMap){
    response_header_t* response;
    uint32_t keySize = header->key_size;
    uint32_t valSize = header->value_size;
    bool works;
    if(keySize < MIN_KEY_SIZE || keySize > MAX_KEY_SIZE){
        response = calloc(1, sizeof(response_header_t));
        response->response_code = BAD_REQUEST;
        response->value_size = 0;
        //write(confd, response, sizeof(request_header_t));
        rio_writen(confd, response, sizeof(response_header_t));
        free(response);
        close(confd);
        return 1;
    }
    if(valSize < MIN_VALUE_SIZE || valSize > MAX_VALUE_SIZE){
        response = calloc(1, sizeof(response_header_t));
        response->response_code = BAD_REQUEST;
        response->value_size = 0;
        //write(confd, response, sizeof(request_header_t));
        rio_writen(confd, response, sizeof(response_header_t));
        free(response);
        close(confd);
        return 1;
    }
    void* keyBase = calloc(1, keySize);
    //error checking
    read(confd, keyBase, keySize);

    map_key_t key;
    key.key_len = keySize;
    key.key_base = keyBase;

    void* valBase = calloc(1, valSize);
    //error checking
    read(confd, valBase, valSize);
    map_val_t value;
    value.val_len = valSize;
    value.val_base = valBase;
    if(theMap->capacity == theMap->size){
        works = put(theMap, key, value, true);
    }
    else{
        //printf("%d\n", theMap->capacity);
        //printf("%d\n", theMap->size);
        works = put(theMap, key, value, false);
    }

    if(works == true){
        response = calloc(1, sizeof(response_header_t));
        response->response_code = OK;
        response->value_size = 0;
        //write(confd, response, sizeof(request_header_t));
        rio_writen(confd, response, sizeof(response_header_t));
    }
    else{
        response = calloc(1, sizeof(response_header_t));
        response->response_code = BAD_REQUEST;
        response->value_size = 0;
        //write(confd, response, sizeof(request_header_t));
        rio_writen(confd, response, sizeof(response_header_t));
        free(keyBase);
        free(valBase);
    }
    free(response);
    close(confd);


    return 0;
}

int getRequest(request_header_t* header, int confd, hashmap_t* theMap){
    response_header_t* response;
    map_val_t cacVal;
    uint32_t keySize = header->key_size;
    if(keySize < MIN_KEY_SIZE || keySize > MAX_KEY_SIZE){
        response = calloc(1, sizeof(response_header_t));
        response->response_code = BAD_REQUEST;
        response->value_size = 0;
        //write(confd, response, sizeof(request_header_t));
        rio_writen(confd, response, sizeof(response_header_t));
        free(response);
        close(confd);
        return 1;
    }
    void* keyBase = calloc(1, keySize);
    //error checking
    read(confd, keyBase, keySize);

    map_key_t key;
    key.key_len = keySize;
    key.key_base = keyBase;
    cacVal = get(theMap, key);
    free(keyBase);

    response = calloc(1, sizeof(response_header_t));
    if(cacVal.val_base == NULL){
        response->response_code = NOT_FOUND;
        response->value_size = 0;
        //write(confd, response, sizeof(request_header_t));
        rio_writen(confd, response, sizeof(response_header_t));
        close(confd);
        free(response);
        return 1;
    }

    response->response_code = OK;
    response->value_size = cacVal.val_len;
    //write(confd, response, sizeof(response_header_t));
    rio_writen(confd, response, sizeof(response_header_t));
    free(response);
    //write(confd, cacVal.val_base, cacVal.val_len);
    rio_writen(confd, cacVal.val_base, cacVal.val_len);
    close(confd);

    return 0;
}

int clearRequest(request_header_t* header, int confd, hashmap_t* theMap){
    bool works;
    works = clear_map(theMap);
    response_header_t* response;
    response = calloc(1, sizeof(response_header_t));
    if(works == true){
        response->response_code = OK;
        response->value_size = 0;
        //write(confd, response, sizeof(response_header_t));
        rio_writen(confd, response, sizeof(response_header_t));
        free(response);
    }
    else{
        response->response_code = BAD_REQUEST;
        response->value_size = 0;
        //write(confd, response, sizeof(response_header_t));
        rio_writen(confd, response, sizeof(response_header_t));
        free(response);
    }
    close(confd);
    return 0;
}

int evictRequest(request_header_t* header, int confd, hashmap_t* theMap){
    response_header_t* response;
    uint32_t keySize = header->key_size;
    if(keySize < MIN_KEY_SIZE || keySize > MAX_KEY_SIZE){
        response = calloc(1, sizeof(response_header_t));
        response->response_code = BAD_REQUEST;
        response->value_size = 0;
        //write(confd, response, sizeof(request_header_t));
        rio_writen(confd, response, sizeof(response_header_t));
        free(response);
        close(confd);
        return 1;
    }
    void* keyBase = calloc(1, keySize);
    //error checking
    read(confd, keyBase, keySize);
    map_key_t key;
    key.key_len = keySize;
    key.key_base = keyBase;
    delete(theMap, key);
    response = calloc(1, sizeof(response_header_t));
    response->response_code = OK;
    response->value_size = 0;
    //write(confd, response, sizeof(request_header_t));
    rio_writen(confd, response, sizeof(response_header_t));
    free(response);
    close(confd);
    return 1;
}