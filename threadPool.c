#include "threadPool.h"

void initThreadPool(pThreadPool_t pPool, int threadNum) {
    pPool->threadNum = threadNum;
    pPool->pid = (pthread_t*)calloc(threadNum, sizeof(pthread_t));
    initQue(&pPool->que);
}

void cleanFunc(void* p) {
    pQue_t pQue = (pQue_t)p;
    pthread_mutex_unlock(&pQue->lock);
}

void* threadFunc(void* p) {
    pQue_t pQue = (pQue_t)p;
    pNode_t pCur;
    int getSucc = 1;
    while(1) {
        pthread_mutex_lock(&pQue->lock);
        pthread_cleanup_push(cleanFunc, pQue);
        if(pQue->size == 0) {
            pthread_cond_wait(&pQue->cond, &pQue->lock);
        }
        getSucc = getQue(pQue, &pCur);
        pthread_mutex_unlock(&pQue->lock);
        if(getSucc == 0) {
            transFile(pCur->clientFd);
            free(pCur);
            pCur = NULL;
        }
        pthread_cleanup_pop(1);
    }
    pthread_exit(NULL);
}

void startThreadPool(pThreadPool_t pPool) {
    for(int i = 0; i < pPool->threadNum; ++i) {
        pthread_create(pPool->pid + i, NULL, threadFunc, &pPool->que);
    }
}

void sigFunc(int sigNum) {
    printf("sig %d is coming!\n", sigNum);
}

void transFile(int clientFd) {
    signal(SIGPIPE, sigFunc);
    Train_t train;
    bzero(&train, sizeof(train));
    int fd = open("file", O_RDWR);
    // storage the len of file name, and send it
    train.len = 4;
    strcpy(train.buf, "file");
    send(clientFd, &train, 4 + train.len, 0);
    struct stat fileInfo;
    bzero(&fileInfo, sizeof(fileInfo));

    fstat(fd, &fileInfo);
    train.len = sizeof(fileInfo.st_size);
    memcpy(train.buf, &fileInfo.st_size, train.len);
    send(clientFd, &train, 4 + train.len, 0);
    printf("filesize = %ld\n", fileInfo.st_size);
    
    // never copy
    int sfd[2];
    pipe(sfd);
    int recvLen = 0;
    while(recvLen < fileInfo.st_size) {
        int ret = splice(fd, 0, sfd[1], 0, 128, 0);
        ret = splice(sfd[0], 0, clientFd, 0, ret, 0);
        recvLen += ret;
    }
}

int initTcp(char* ip, char* port, int *sockFd) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd == -1) {
        perror("socket eror");
        return -1;
    }

    struct sockaddr_in serAddr;
    bzero(&serAddr, sizeof(serAddr));
    serAddr.sin_family = AF_INET;
    serAddr.sin_addr.s_addr = inet_addr(ip);
    serAddr.sin_port = htons(atoi(port));

    int reuse = 1; // once greater than 0, set options take effect
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    int ret = -1;
    ret = bind(sfd, (struct sockaddr*)&serAddr, sizeof(serAddr));
    if(ret == -1) {
        perror("bind error");
        return -1;
    }

    ret = listen(sfd, 10);
    if(ret == -1) {
        perror("bind error");
        return -1;
    }

    *sockFd = sfd;
    return 0;
}

void epollAddFd(int fd, int epFd) {
    struct epoll_event event;
    bzero(&event, sizeof(event));

    event.events = EPOLLIN;
    event.data.fd = fd;
    epoll_ctl(epFd, EPOLL_CTL_ADD, fd, &event);
}
