#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "workQue.h"

#define BUFSIZE 1024

typedef struct {
    int len;
    char buf[BUFSIZE];
}Train_t;

typedef struct {
    int threadNum;
    pthread_t *pid;
    Que_t que;
}threadPool_t, *pThreadPool_t;

void initThreadPool(pThreadPool_t pPool, int threadNum);
void startThreadPool(pThreadPool_t pPool);
void transFile(int clientFd);
int initTcp(char* ip, char* port, int *sockFd);
void epollAddFd(int fd, int epFd);

#endif
