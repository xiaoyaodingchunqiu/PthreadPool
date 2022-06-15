#include "workQue.h"

void initQue(pQue_t pQue) {
    pQue->size = 0;
    pQue->pHead = NULL;
    pQue->pTail = NULL;
    pthread_mutex_init(&pQue->lock, NULL);
    pthread_cond_init(&pQue->cond, NULL);
}

void insertQue(pQue_t pQue, pNode_t pNew) {
    if(pQue->pTail == NULL) {
        pQue->pHead = pNew;
        pQue->pTail = pNew;
    } else {
        pQue->pTail->pNext = pNew;
        pQue->pTail = pNew;
    }
    ++pQue->size;
}

int getQue(pQue_t pQue, pNode_t *pGet) {
    if(pQue->size == 0) {
        return -1;
    }

    *pGet = pQue->pHead;
    pQue->pHead = pQue->pHead->pNext;
    --pQue->size;
    if(pQue->size == 0) {
        pQue->pTail = NULL;
    }
    return 0;
}
