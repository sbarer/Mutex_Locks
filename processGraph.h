//
//  processGraph.h
//  prj4
//
//  Created by Simon Barer on 2018-11-01.
//  Copyright © 2018 Simon Barer. All rights reserved.
//

#ifndef processGraph_h
#define processGraph_h

#include <stdbool.h>
#include <stdio.h>
#include "klock.h"

typedef enum { PROCESS, MUTEX } NodeType;

typedef struct Edge {
    struct Edge* next;
    int GI_ID;
} Edge;

typedef struct Node {
    NodeType type;
    struct Edge* next;
    int graphIndex;
    int lockNum;
    pthread_t threadID;
} Node;

void createNodeP(NodeType type);

void createNodeM(NodeType type, int number);

Node* returnNodeP(void);

Node* returnNodeM(int number);

bool checkNode(pthread_t self);

void addEdge(Node* from, Node* to);

void deleteEdge(Node* from, Node* to);

// returns false if a cycle is not found
// returns true if a cycle is found
bool findCycleHelper(Node* start, int target, int length);
bool findCycle(Node* start, int target, int length);

void printGraph(void);

void graphCleanup(void);

// |-> [Process] -> [Mutex] -> [Process] -> [Mutex] -|

#endif /* processGraph_h */
