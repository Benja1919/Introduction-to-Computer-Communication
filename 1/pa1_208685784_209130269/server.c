#include "server.h"

void enqueue(queue* q, char* value){
    q_node* node = (q_node*)malloc(sizeof(q_node));
    node->value = (char*)malloc(sizeof(strlen(value)+1));
    strcpy(node->value, value);
    node->next = NULL;
    if (q->size == 0)
    {
        q->header = node;
        q->last = node;
        q->size++;
    }
    else{
        q->last->next = node;
        q->last = node;
        q->size++;
    }
}

char* dequeue(queue* q){
    q_node* cur_node;
    if (q->size == 0)
    {
        return NULL;
    }
    cur_node = q->header;
    char* value = cur_node->value;
    q->header = cur_node->next;
    q->size--;
    free(cur_node);
    return value;
}

double mu;
SOCKET connection_socket;
HANDLE mutex;
boolean is_closed;
FILE *tmp_file;
int run_id, seed, QSize;

int main(int argc, char const *argv[])
{
    int port, status, received_size, val;
    double t;
    WSADATA wsaData;
    SOCKET listen_socket;
    struct sockaddr_in my_addr, connection_addr;
    char job[MAX_LEN], tmp[MAX_LEN];
    is_closed = FALSE;
    queue *q = (queue*)malloc(sizeof(queue));
    q->size = 0;
    q->header = NULL;
    q->last = NULL;

    if (argc != 6)
    {
        perror("Invalid number of arguments");
        return 1;
    }
    srand(seed);
    tmp_file = fopen("server_tmp.log", "w");
    port = atoi(argv[1]);
    seed = atoi(argv[2]);
    run_id = atoi(argv[3]);
    mu = atof(argv[4]);
    QSize = atoi(argv[5]);
    if ((port == 0 && strcmp(argv[1], "0") != 0) || port < 0 || port > 65535)
    {
        perror("Invalid port\n");
        return 1;
    }
    if ((seed == 0 && strcmp(argv[2], "0") != 0) || seed < 0 || seed > 32767)
    {
        perror("Invalid seed\n");
        return 1;
    }
    if (run_id <= 0)
    {
        perror("Invalid run_id\n");
        return 1;
    }
    if (mu <= 0)
    {
        perror("Invalid mu\n");
        return 1;
    }
    if (QSize <= 0)
    {
        perror("Invalid QSize\n");
        return 1;
    }
    status = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (status != NO_ERROR)
    {
        perror("Error at WSAStartup()\n");
        return 1;
    }
    listen_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket == INVALID_SOCKET)
    {
        perror("Error at socket()\n");
        return 1;
    }
    setsockopt(listen_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof(int));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);
    status = bind(listen_socket, (SOCKADDR*)&my_addr, sizeof(my_addr));
    if (status != 0)
    {
        perror("Error at bind()\n");
        return 1;
    }
    status = listen(listen_socket, 1);
    if (status != 0)
    {
        perror("Error at listen()\n");
        return 1;
    }
    connection_socket = accept(listen_socket, (struct sockaddr*)&connection_addr, NULL);
    setsockopt(connection_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof(int));
    mutex = CreateMutex(NULL, FALSE, NULL);
    CreateThread(NULL, 0, job_handler, q, 0, NULL);
    while (1)
    {
        received_size = recv(connection_socket, (char*)&job, MAX_LEN, 0);
        if (received_size == 0)
        {
            is_closed = TRUE;
            break;
        }
        
        if (q->size == QSize)
        {
            sprintf(tmp, "%c%s", 'f', job);
            send(connection_socket, (char*)&tmp, strlen((char*)&tmp), 0);
        }
        else{
            WaitForSingleObject(mutex, INFINITE);
            enqueue(q, job);
            fprintf(tmp_file, "%f %d\n", (double)(clock())/CLOCKS_PER_SEC, q->size);
            ReleaseMutex(mutex);
        }
    }
    ExitThread(0);
}

DWORD WINAPI job_handler(void* data){
    queue* q = (queue*)data;
    double t;
    char cur_job[MAX_LEN], file_name[20], *tmp, ch;
    FILE *log_file;
    while (!is_closed || q->size != 0)
    {
        if (q->size != 0)
        {
            WaitForSingleObject(mutex, INFINITE);
            tmp = dequeue(q);
            fprintf(tmp_file, "%f %d\n", (double)(clock())/CLOCKS_PER_SEC, q->size);
            ReleaseMutex(mutex);
            t = exp_wait_time(mu);
            Sleep(t);
            sprintf(cur_job, "%c%s", 's', tmp);
            send(connection_socket, cur_job, strlen(cur_job), 0);
        }
    }
    
    fclose(tmp_file);
    tmp_file = fopen("server_tmp.log", "r");
    sprintf(file_name, "server_%d.log", run_id);
    log_file = fopen(file_name, "w");
    fprintf(log_file, "server_%d.log: seed=%d, mu=%f, QSize=%d\n", run_id, seed, mu, QSize);
    fprintf(stderr, "server_%d.log: seed=%d, mu=%f, QSize=%d\n", run_id, seed, mu, QSize);
    while((ch = fgetc(tmp_file)) != EOF)
        fputc(ch, log_file);
    fclose(tmp_file);
    fclose(log_file);
    remove("server_tmp.log");
    ExitThread(0);
}

double exp_wait_time(double mu)
{
    double u = (double)rand()/(double)RAND_MAX;
    return -1000*log(u)/mu;
}