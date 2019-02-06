//
//  processGraph.c
//  prj4
//
//  Created by Simon Barer on 2018-11-01.
//  Copyright © 2018 Simon Barer. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include "processGraph.h"

#define NO_THREAD 0
#define No_ID -1

static int nodeCount = 0;
Node* graph;
pthread_mutex_t graphAccess = PTHREAD_MUTEX_INITIALIZER;

void createNodeM(NodeType type, int number) {
    pthread_mutex_lock(&graphAccess);
    // increment number of nodes
    nodeCount++;
    // allocate space in graph for new node
    graph = realloc(graph, sizeof(Node) * nodeCount);
    
    // Define new node in graph
    Node* newNode = &graph[nodeCount-1];
    
    // populate node's info
    newNode->graphIndex = nodeCount-1;
    newNode->lockNum = number;
    newNode->threadID = NO_THREAD;
    newNode->next = NULL;
    newNode->type = type;
    
    printf("Mutex %d = Node %d\n", newNode->lockNum, newNode->graphIndex);
    pthread_mutex_unlock(&graphAccess);
}

void createNodeP(NodeType type) {
    pthread_mutex_lock(&graphAccess);
    // increment number of nodes
    nodeCount++;
    // allocate space in graph for new node
    graph = realloc(graph, sizeof(Node) * nodeCount);
    
    // Define new node in graph
    Node* newNode = &graph[nodeCount-1];
    
    // populate node's info
    newNode->graphIndex = nodeCount-1;
    newNode->lockNum = No_ID;
    newNode->threadID = pthread_self();
    newNode->next = NULL;
    newNode->type = type;
    
    printf("Thread %ld = Node %d\n", newNode->threadID, newNode->graphIndex);
    pthread_mutex_unlock(&graphAccess);
}

Node* returnNodeP(void) {
    pthread_mutex_lock(&graphAccess);
    for (int i = 0; i < nodeCount; i++) {
        if (graph[i].threadID == pthread_self()) {
            pthread_mutex_unlock(&graphAccess);
            return &graph[i];
        }
    }
    pthread_mutex_unlock(&graphAccess);
    return NULL;
}

Node* returnNodeM(int number) {
    pthread_mutex_lock(&graphAccess);
    for (int i = 0; i < nodeCount; i++) {
        if (graph[i].lockNum == number) {
            pthread_mutex_unlock(&graphAccess);
            return &graph[i];
        }
    }
    pthread_mutex_unlock(&graphAccess);
    return NULL;
}


bool checkNode(pthread_t self) {
    pthread_mutex_lock(&graphAccess);
    for (int i = 0; i < nodeCount; i++) {
        if (graph[i].threadID == pthread_self()) {
            pthread_mutex_unlock(&graphAccess);
            return true;
        }
    }
    pthread_mutex_unlock(&graphAccess);
    return false;
}

void addEdge(Node* from, Node* to) {
    pthread_mutex_lock(&graphAccess);
    
    // check to see if edge already exists
    Edge* curr = from->next;
    while(curr != NULL) {
        if (curr->GI_ID == to->graphIndex) {
            pthread_mutex_unlock(&graphAccess);
            return;
        }
        curr = curr->next;
    }
    
    // create edge struct for new connection
    Edge* newEdge = malloc(sizeof(Edge));
    newEdge->next = NULL;
    newEdge->GI_ID = to->graphIndex;
    
    // add edge to corresponding node
    if (from->next == NULL) {
        from->next = newEdge;
    } else {
        Edge* curr = from->next;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = newEdge;
    }
    
    printf("Edge created from Node %d -> Node %d\n",
           from->graphIndex, to->graphIndex);
    pthread_mutex_unlock(&graphAccess);
    printGraph();
    
}

void deleteEdge(Node* from, Node* to) {
    pthread_mutex_lock(&graphAccess);
    Edge* curr = from->next;
    Edge* prev = NULL;
    
    // if only one element in linked list
    if (curr->next == NULL) {
        from->next = NULL;
        // delete edge
        free(curr);
        pthread_mutex_unlock(&graphAccess);
        printGraph();
        return;
    }
    
    // find edge to be removed
    while (curr->GI_ID != to->lockNum) {
        prev = curr;
        curr = curr->next;
    }
    
    // extricate edge for deletion
    prev->next = curr->next;
    
    // delete edge
    free(curr);
    
    printf("Edge deleted from Node %d -> Node %d\n",
           from->graphIndex, to->graphIndex);
    pthread_mutex_unlock(&graphAccess);
    printGraph();
}

bool findCycleHelper(Node* start, int target_index, int length) {
    pthread_mutex_lock(&graphAccess);
    bool ret = findCycle(start, target_index, length);
    pthread_mutex_unlock(&graphAccess);
    return ret;
}

bool findCycle(Node* start, int target_index, int length) {
    // return true if cycle is found
    if (start->graphIndex == target_index && length > 0) {
        return true;
    }
    
    // recursively call cycle for each edge (depth first search)
    int len = length + 1;
    Edge* curr = start->next;
    while (curr != NULL) {
        int nextCheck = curr->GI_ID;
        printf("\tchecking edge between node %d and node %d\n",
               start->graphIndex, nextCheck);
        if (findCycle(&graph[nextCheck], target_index, len) == true) {
            return true;
        }
        curr = curr->next;
    }
    
    printf("\t\treached dead end at node %d\n", start->graphIndex);
    return false;
}

void printGraph() {
    pthread_mutex_lock(&graphAccess);
    for (int i = 0; i < nodeCount; i++) {
        printf("[Node %d] -> ", i);
        Edge* edge = graph[i].next;
        while (edge != NULL) {
            printf("[%d] -> ", edge->GI_ID);
            edge = edge->next;
        }
        printf("NULL\n");
    }
    pthread_mutex_unlock(&graphAccess);
}


void graphCleanup() {
    pthread_mutex_lock(&graphAccess);
    for (int i = 0; i < nodeCount; i++) {
        // free all edges
        while (graph[i].next != NULL) {
            Edge* curr = graph[i].next;
            graph[i].next = graph[i].next->next;
            free(curr);
        }
    }
    // free graph
    free(graph);
    pthread_mutex_unlock(&graphAccess);
}
