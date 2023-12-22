#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INPUT 255

/*Structs*/

/*Node struct used in a linked list queue struct, where each node represents a packet in the queue*/
typedef struct _q_node {
    int arrival_time; /*Packet's arrival time*/
    struct _q_node* next; /*Pointer to the node of the next packet in the queue*/
} q_node;

/*Queue struct used for packets fifo queues*/
typedef struct _queue {
    q_node* header; /*Pointer to the node of the first packet in the queue*/
    q_node* last; /*Pointer to the node of the last packet in the queue*/
    int size; /*The size of the queue*/
} queue;

/*Port struct used to store specific variables of each port*/
typedef struct _port {
    bool* requests; /*A boolean array where a value for an index is true iff the index'th input port sends a request to this output port*/
    bool* grants; /*A boolean array where a value for an index is true iff the index'th output port sends a grand to this input port*/
    int request_priority; /*The current priorotized input port index for requests to this output port*/
    int grant_priority; /*The current priorotized output port index for grants to this input port*/
    bool input_linked; /*True iff this input port has been linked to an output port*/
    bool output_linked; /*True iff this output port has been linked to an input port*/
} port;

/*function to enqueue a packet to a packet queue*/
void enqueue(queue *q, int arrival_time){
    q_node* node = (q_node*)malloc(sizeof(q_node));
    if (node == NULL)
    {
        perror("Failed memory allocation");
        exit(-1);
    }
    node->arrival_time = arrival_time;
    node->next = NULL;
    if (q->size == 0) /*If the queue is empty*/
    {
        q->header = node; /*Mark the new node as the first node in the queue*/
        q->last = node; /*Mark the new node as the last node in the queue*/
        q->size++; /*Increase queue size*/
    }
    else{ /*The queue isn't empty*/
        q->last->next = node; /*Set the new node as the next node of the previous last node*/
        q->last = node; /*Set the new node as the last node in the queue*/
        q->size++; /*Increase queue size*/
    }
}

/*function to dequeue a packet from a packet fifo queue. Should only be called if the queue isn't empty*/
int dequeue(queue *q){
    q_node* cur_node;
    if (q->size == 0) /*If the queue is empty*/
    {
        perror("Deqeueing an empty queue"); /*Print an error and exit*/
        exit(-1);
    }
    cur_node = q->header; /*The queue the first node in the queue*/
    int arrival_time = cur_node->arrival_time;
    q->header = cur_node->next; /*Set the next node as first node in the queue*/
    q->size--; /*Decrease queue size*/
    free(cur_node);
    return arrival_time;
}

int main(int argc, char const *argv[])
{
    /*intialize arguments*/
    int N, k, r, cur_time, arr_time, arr_port, dest_port;
    char cur_input[MAX_INPUT], file_name[MAX_INPUT];
    bool queues_empty = false, eof = false;
    queue ***queues;
    port *ports;
    FILE *r_log;

    if (argc != 4) /*If number of argument isn't 4*/
    {
        perror("Invalid number of arguments"); /*Print an error and exit*/
        exit(-1);
    }
    N = atoi(argv[1]);
    k = atoi(argv[2]);
    r = atoi(argv[3]);

    /*argument format checking, if error - perror and exit with -1*/

    /*check format of N*/
    if (N <= 0)
    {
        perror("Invalid N\n");
        exit(-1);
    }
    /*check format of k*/
    if (k <= 0)
    {
        perror("Invalid k\n");
        exit(-1);
    }
    /*check format of r*/
    if ((r == 0 && strcmp(argv[3], "0") != 0))
    {
        perror("Invalid r\n");
        exit(-1);
    }

    /*allocate queues, if assertion failes, perror and exit with -1*/
    queues = (queue***)malloc(N * sizeof(queue**));
    if (queues == NULL) {
        perror("Failed memory allocation");
        exit(-1);
    }
    /*allocate arrays within queues, if assertion failes, perror and exit with -1*/
    for (int i = 0; i < N; i++) {
        queues[i] = (queue**)malloc(N * sizeof(queue*));
        if (queues[i] == NULL) {
            perror("Failed memory allocation");
            exit(-1);
        }
        for (int j = 0; j < N; j++)
        {
            queues[i][j] = (queue*)malloc(sizeof(queue));
            queues[i][j]->size = 0;
        }
    }
    /*allocate ports, if assertion failes, perror and exit with -1*/
    ports = (port*)malloc(N * sizeof(port));
    if (ports == NULL) {
        perror("Failed memory allocation");
        exit(-1);
    }
    /*for each port, allocation of grants and requests*/
    for (int i = 0; i < N; i++) {
        ports[i].grants = (bool*)malloc(N * sizeof(bool));
        if (ports[i].grants == NULL) {
            perror("Failed memory allocation");
            exit(-1);
        }
        ports[i].requests = (bool*)malloc(N * sizeof(bool));
        if (ports[i].requests == NULL) {
            perror("Failed memory allocation");
            exit(-1);
        }
    }
    /*intialize r.log file*/
    sprintf(file_name, "%d.log", r);
    r_log = fopen(file_name, "w");

    for (int i = 0; i < N; i++)
    /*intialize priorities of all queues*/
    {
        ports[i].grant_priority = 0;
        ports[i].request_priority = 0;
    }
    /*initialize cur_time*/
    cur_time = 0;

    fgets(cur_input, MAX_INPUT, stdin); /*Read the first packet from stdin*/
    sscanf(cur_input, "%d %d %d", &arr_time, &arr_port, &dest_port); /*Parse the read packet into relavent variables*/

    while (!queues_empty || !eof) /*Loop until all queues are empty, and stdin has reached EOF*/
    {
        while (!eof && arr_time <= cur_time) /*read until eof reached or last read packet's arrival time is bigger than current time (meaning it didn't arrive yet and shouldn't be inserted to the queues)*/
        {
            enqueue(queues[arr_port][dest_port], arr_time); /*Enqueue the last read packet*/
            if(fgets(cur_input, MAX_INPUT, stdin) == NULL) /* Try reading the next packet from stdin*/
            {
                eof = true; /*If the reading failed, eof of stdin has been reached*/
                break;
            }
            sscanf(cur_input, "%d %d %d", &arr_time, &arr_port, &dest_port); /* Parse the read packet into relavent variables*/
        }

        queues_empty = true; /*Initialize queues_empty as true*/
        for (int i = 0; i < N; i++)
        /*Initialize all queues as not linked*/
        {
            ports[i].input_linked = false;
            ports[i].output_linked = false;
        }

        for (int cur_iter = 0; cur_iter < k; cur_iter++) /*Iterate k times*/
        {
            
            for (int i = 0; i < N; i++)
            {
                /*Set all grants and request arrays to false*/
                memset(ports[i].grants, false, N*sizeof(bool));
                memset(ports[i].requests, false, N*sizeof(bool));
            }

            for (int cur_output = 0; cur_output < N; cur_output++) /*Iterate through the output ports*/
            {
                if (!ports[cur_output].output_linked) /*if the current output isn't linked yet*/
                {
                    for (int cur_input = 0; cur_input < N; cur_input++) /*Iterate through the input ports*/
                    {
                        if (!ports[cur_input].input_linked) /*if the current intput isn't linked yet*/
                        {
                            if(queues[cur_input][cur_output]->size > 0) /*If the queue for the current input and output isn't empty*/
                            {
                                queues_empty = false; /*Mark that the queues aren't empty*/
                                ports[cur_output].requests[cur_input] = true;  /*Raise a request from the current input port to the current output port*/
                            }
                        }
                    }

                    /*Choosing an input port to send a grant to*/
                    for (int i = 0; i < N; i++)
                    {
                        int cur_input = (ports[cur_output].request_priority + i) % N; /*Iterate through all input ports, starting from the port with most priority*/
                        if (ports[cur_output].requests[cur_input]) /*If the current input port sent a request*/
                        {
                            ports[cur_input].grants[cur_output] = true; /*Send a grant to the current input port*/
                            ports[cur_output].request_priority = cur_input; /*Mark the current input port as the one with most priority in case this output port wasn't accepted*/
                            break; /*Found a port to grant to*/
                        }
                        
                    }
                }
            }

            for (int cur_input = 0; cur_input < N; cur_input++) /*Iterate through the input ports*/
            {
                if (!ports[cur_input].input_linked) /*if the current input isn't linked yet*/
                {
                    /*Choosing an output port to accept*/
                    for (int i = 0; i < N; i++)
                    {
                        int cur_output = (ports[cur_input].grant_priority + i) % N; /*Iterate through all output ports, starting from the port with most priority*/
                        if (ports[cur_input].grants[cur_output]) /*If the current output port sent a grant*/
                        {
                            ports[cur_input].input_linked = true; /*Set the current input port as linked*/
                            ports[cur_output].output_linked = true; /*Set the current output port as linked*/
                            ports[cur_input].grant_priority = (cur_output + 1) % N; /*Set the current input port as the one with least priority*/
                            ports[cur_output].request_priority = (cur_input + 1) % N; /*Set the current output port as the one with least priority*/
                            printf("%d %d %d %d\n", dequeue(queues[cur_input][cur_output]), cur_input, cur_output, cur_time); /*Print the current link information*/
                            break;
                        }
                        
                    }
                    
                }
            }
        }

        if (!queues_empty || !eof) /*If it is not the final loop*/
        {
            for (int cur_input = 0; cur_input < N; cur_input++) /*Iterate through all input ports*/
            {
                for (int cur_output = 0; cur_output < N; cur_output++) /*Iterate through all output ports*/
                {
                    fprintf(r_log, "%d %d %d %d\n", cur_time, cur_input, cur_output, queues[cur_input][cur_output]->size); /*Print the current buffer size information to r_log*/
                }
                
            }
        }
        cur_time++; /*Increase the time by 1*/
    }

    /*free memory allocation*/
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            free(queues[i][j]);
        }
        free(queues[i]);
        free(ports[i].grants);
        free(ports[i].requests);
    }
    free(queues);
    free(ports);
    fclose(r_log);
    
    exit(0);
}
