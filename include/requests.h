#ifndef REQUESTS_H
#define REQUESTS_H
#include "cream.h"
//#include "hashmap.h"
#include "utils.h"

int getRequest(request_header_t* header, int confd, hashmap_t* theMap);
int putRequest(request_header_t* header, int confd, hashmap_t* theMap);
int clearRequest(request_header_t* header, int confd, hashmap_t* theMap);
int evictRequest(request_header_t* header, int confd, hashmap_t* theMap);
//void errorResponse(int confd);





#endif