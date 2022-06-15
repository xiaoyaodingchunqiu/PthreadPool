#include "threadPool.h"

int exitPipe[2];

void sigFunc(int sigNum) {
    printf("sig %d is comming\n", sigNum);
    write(exitPipe[1], &sigNum, 4);
}

int main(int argc, char* argv[])
{
    pipe(exitPipe);

    if(fork()) { // if dad process
        close(exitPipe[0]);
        signal(SIGUSR1, sigFunc);
        wait(NULL);
        printf("child exit\n");
        exit(0);
    }

    close(exitPipe[1]);

    // create and init
    int threadNum = atoi(argv[3]);
    threadPool_t pool;
    bzero(&pool, sizeof(pool));
    initThreadPool(&pool, threadNum);

    startThreadPool(&pool);
    int sfd = 0;
    initTcp(argv[1], argv[2], &sfd);

    int epFd = epoll_create(1);

    epollAddFd(sfd, epFd);
    epollAddFd(exitPipe[0], epFd);
    struct epoll_event ev[2];

    int newFd = 0;
    int readyNum = 0;
    while(1) {
        readyNum = epoll_wait(epFd, ev, 2, -1);
        for(int i = 0; i < readyNum; ++i) {
            if(ev[i].data.fd == sfd) {
                newFd = accept(sfd, NULL, NULL);
                pNode_t pNew = (pNode_t)calloc(1, sizeof(Node_t));
                pthread_mutex_lock(&pool.que.lock);
                pNew->clientFd = newFd;
                insertQue(&pool.que, pNew);
                pthread_cond_signal(&pool.que.cond);
                pthread_mutex_unlock(&pool.que.lock);
            } else if(ev[i].data.fd == exitPipe[0]) {
                // do exit
                for(int j = 0; j < threadNum; ++j) {
                    pthread_cancel(pool.pid[j]);
                }
                for(int j = 0; j < threadNum; ++j) {
                    pthread_join(pool.pid[j], NULL);
                }
                exit(0);
            }
        }
    }
    return 0;
}

