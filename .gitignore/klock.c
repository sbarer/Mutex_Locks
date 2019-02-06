#include "klock.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include "processGraph.h"

#define NO_LOCK -1
#define NO_THREAD 0

int numOfLocks = 0;
SmartLock* lockLedger;

void init_lock(SmartLock* lock) {
    printf("attempting to initialize lock %d\n", numOfLocks);
    
    lockLedger = realloc(lockLedger,
                         sizeof(SmartLock) * (numOfLocks + 1));
    if (lockLedger == NULL) {
        printf("unable to allocate memory\n");
        return;
    }
    pthread_mutex_init(&(lock->mutex), NULL);
    
    lock->num = numOfLocks;
    lockLedger[numOfLocks] = *lock;
    
    // create node for new lock
    createNodeM(MUTEX, lock->num);
    
    numOfLocks++;
}

int lock(SmartLock* lock) {
    bool ret = false;
    printf("Thread %ld is attempting to obtain lock: %d\n", pthread_self(), lock->num);
    
    // check to see if node has been created for this thread
    // if not, create one
    printf("checking if node has been created\n");
    if (checkNode(pthread_self()) == false) {
        printf("Creating node for %ld\n", pthread_self());
        createNodeP(PROCESS);
    }
    
    // find nodes we wish to connect
    printf("returnNodeP\n");
    Node* Process = returnNodeP();
    printf("returnNodeM\n");
    Node* Mutex = returnNodeM(lock->num);
    
    // add edge, check if cycle forms
    // if cycle forms, edge is invalid, so remove it
    // if no cycle is formed, the edge is valid,
    // so give the lock to the calling process
    printf("Creating edge for %ld\n", pthread_self());
    addEdge(Process, Mutex);
    
    if (findCycleHelper(Process, Process->graphIndex, 0) == true) {
        printf("\tCycle found.\n");
    }
    // Otherwise, provide lock to thread
    else {
//        while (Mutex->next != NULL);
        printf("\tNo cycle found.\n");
        pthread_mutex_lock(&(lock->mutex));
        printf("\tGiving lock %d to Thread %ld.\n", lock->num, pthread_self());
        deleteEdge(Process, Mutex);
        addEdge(Mutex, Process);
        ret = true;
    }
    
    return ret;
}


void unlock(SmartLock* lock) {
    printf("attempting to unlock, for lock: %d\n", lock->num);

    // find nodes we wish to diconnect
    Node* Process = returnNodeP();
    Node* Mutex = returnNodeM(lock->num);
    
    deleteEdge(Mutex, Process);
        
	pthread_mutex_unlock(&(lock->mutex));
}

/*
 * Cleanup any dynamic allocated memory for SmartLock to avoid memory leak
 * You can assume that cleanup will always be the last function call
 * in main function of the test cases.
 */
void cleanup() {
    graphCleanup();
    free(lockLedger);
    printf("\tThreads have executed succesfully and all memory has been freed.\n");
}
