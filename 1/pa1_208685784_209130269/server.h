#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <math.h>
#include <time.h>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")

#define MAX_LEN 4096

typedef struct _q_node {
    char* value;
    struct _q_node* next;
} q_node;

typedef struct _queue {
    q_node* header;
    q_node* last;
    int size;
} queue;

void enqueue(queue* q, char* value);
char* dequeue(queue* q);
double exp_wait_time(double mu);
DWORD WINAPI job_handler(void* data);