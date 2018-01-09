#ifndef HELPERS_H
#define HELPERS_H
#include <stdlib.h>
#include <stdio.h>

int open_listenfd(char* port);
ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void helpmenu();

#endif