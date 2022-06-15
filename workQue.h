#ifndef __WORKQUE_H__
#define __WORKQUE_H__

#include <pthread.h>

// Task node
typedef struct node {
    int clientFd;
    struct node *pNext;
}Node_t, *pNode_t;

typedef struct {
    int size;
    pNode_t pHead, pTail;
    pthread_mutex_t lock;
    pthread_cond_t cond;
}Que_t, *pQue_t;

void initQue(pQue_t pQue);
void insertQue(pQue_t pQue, pNode_t pNew);
int getQue(pQue_t pQue, pNode_t* pGet);

#endif
