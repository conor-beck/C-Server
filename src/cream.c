#include "cream.h"
#include "utils.h"
//#include "hashmap.h"
#include "queue.h"
#include "destruction.h"
#include <getopt.h>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "helpers.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "requests.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

int opterr;
int optopt;
int optind;
char *optarg;

void* thread(void* vargp);
hashmap_t* theMap;
queue_t* theQueue;

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
    pthread_t tid;
    char option;
    int listenfd;
    int* item;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    while((option = getopt(argc, argv, "h")) != -1){
        switch(option){
            case 'h': {
                helpmenu();
                exit(EXIT_SUCCESS);
            }
        }
    }

    if(argc < 4){
        helpmenu();
        exit(EXIT_FAILURE);
    }

    int numWorkers = atoi(argv[1]);
    int portNum = atoi(argv[2]);
    int maxEntries = atoi(argv[3]);
    if(numWorkers == 0){
        helpmenu();
        exit(EXIT_FAILURE);
    }
    if(portNum == 0){
        helpmenu();
        exit(EXIT_FAILURE);
    }
    if(maxEntries == 0){
        helpmenu();
        exit(EXIT_FAILURE);
    }


    theMap = create_map(maxEntries, jenkins_one_at_a_time_hash, mapDestroy);
    //theMap = create_map(maxEntries, testingHash, mapDestroy);
    //int k = 5;
    //int n = 66;
    //int* kb = calloc(1, sizeof(k));
    //int* vb = calloc(1, sizeof(n));
    //*kb = k;
    //*vb = n;
    //map_key_t key;
    //key.key_len = sizeof(k);
    //key.key_base = kb;
    //map_val_t value;
    //value.val_len = sizeof(n);
    //value.val_base = vb;
    //put(theMap, key, value, );
    theQueue = create_queue();
    for(int i = 0; i < numWorkers; i++){
        if(pthread_create(&tid, NULL, thread, NULL) != 0){
            //perror("Thread error\n");
            abort();
        }
    }
    listenfd = open_listenfd(argv[2]);


    while(1){
        clientlen = sizeof(struct sockaddr_storage);
        item = malloc(sizeof(int));
        *item = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        enqueue(theQueue, item);
    }

    exit(0);
}



void* thread(void* vargp){
    //if(pthread_detach(pthread_self()) != 0){
       // perror("Thread detach error\n");
    //}
    int writeRet = 0;
    while(1){
        int* confdPtr = dequeue(theQueue);
        int confd = *confdPtr;
        request_header_t* buf = calloc(1, sizeof(request_header_t));
        writeRet = rio_readn(confd, buf, sizeof(request_header_t));
        if(writeRet == -1){
            close(confd);
            //free(buf);

        }
        //if(read(confd, buf, sizeof(request_header_t)) == -1){
            //error

        //}
        else if(buf->request_code == PUT){

            putRequest(buf, confd, theMap);
        }
        else if(buf->request_code == GET){

            getRequest(buf, confd, theMap);
        }
        else if(buf->request_code == EVICT){
            evictRequest(buf, confd, theMap);
        }
        else if(buf->request_code == CLEAR){
            clearRequest(buf, confd, theMap);
        }
        else{
            response_header_t* response;
            response = calloc(1, sizeof(response_header_t));
            response->response_code = UNSUPPORTED;
            response->value_size = 0;
            //write(confd, response, sizeof(response_header_t));
            rio_writen(confd, response, sizeof(response_header_t));
            free(response);
            close(confd);
        }
        free(buf);
        }
    return NULL;
}
