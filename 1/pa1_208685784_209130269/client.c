#include "client.h"


SOCKET connection_socket;
HANDLE mutex;
int total_drops = 0, total_threads = 0;
FILE *tmp_file;

int main(int argc, char const *argv[])
{
    int port, seed, run_id, status, data_size, val = 1, T, total_packets = 0;
    double lambda, t;
    WSADATA wsaData;
    FILE *log_file;
    struct sockaddr_in connection_addr;
    char job[MAX_LEN], ret[10], file_name[20], ch;
    const char *server_ip;
    
    if (argc != 7)
    {
        perror("Invalid number of arguments");
        return 1;
    }
    server_ip = argv[1];
    port = atoi(argv[2]);
    seed = atoi(argv[3]);
    run_id = atoi(argv[4]);
    lambda = atof(argv[5]);
    T = atoi(argv[6]);
    if ((port == 0 && strcmp(argv[2], "0") != 0) || port < 0 || port > 65535)
    {
        perror("Invalid port\n");
        return 1;
    }
    if ((seed == 0 && strcmp(argv[3], "0") != 0) || seed < 0 || seed > 32767)
    {
        perror("Invalid seed\n");
        return 1;
    }
    if (run_id <= 0)
    {
        perror("Invalid run_id\n");
        return 1;
    }
    if (lambda <= 0)
    {
        perror("Invalid lambda\n");
        return 1;
    }
    if (T <= 0)
    {
        perror("Invalid T\n");
        return 1;
    }
    status = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (status != NO_ERROR)
    {
        perror("Error at WSAStartup()\n");
    }
    connection_socket = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(connection_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof(int));
    if (connection_socket == INVALID_SOCKET)
    {
        perror("Error at socket()\n");
    }
    connection_addr.sin_family = AF_INET;
    connection_addr.sin_addr.s_addr = inet_addr(server_ip);
    connection_addr.sin_port = htons(port);
    srand(seed);
    mutex = CreateMutex(NULL, FALSE, NULL);
    tmp_file = fopen("client_tmp.log", "w");
    status = connect(connection_socket, (struct sockaddr*)&connection_addr, sizeof(connection_addr));
    
    while ((double)(clock())/CLOCKS_PER_SEC <= T)
    {
        sprintf(job, "%f\n", (double)(clock())/CLOCKS_PER_SEC);
        data_size = send(connection_socket, (char*)&job, (int)strlen((char*)&job), 0);
        total_packets++;
        WaitForSingleObject(mutex, INFINITE);
        total_threads++;
        ReleaseMutex(mutex);
        CreateThread(NULL, 0, job_creator, &t, 0, NULL);
        t = exp_wait_time(lambda);
        Sleep(t);
    }
    shutdown(connection_socket, SD_SEND);
    while (total_threads != 0)
    {}
    fclose(tmp_file);
    tmp_file = fopen("client_tmp.log", "r");
    sprintf(file_name, "client_%d.log", run_id);
    log_file = fopen(file_name, "w");
    fprintf(log_file, "client_%d.log: seed=%d, lambda=%f, T=%d, total_pkts=%d, total_drops=%d\n", run_id, seed, lambda, T, total_packets, total_drops);
    fprintf(stderr, "client_%d.log: seed=%d, lambda=%f, T=%d, total_pkts=%d, total_drops=%d\n", run_id, seed, lambda, T, total_packets, total_drops);
    while((ch = fgetc(tmp_file)) != EOF)
        fputc(ch, log_file);
    fclose(tmp_file);
    fclose(log_file);
    remove("client_tmp.log");
    ExitThread(0);
}

DWORD WINAPI job_creator(void* data){
    double t = *(double*)data, creation_time, finish_time;
    int data_size;
    char job[MAX_LEN], tmp[MAX_LEN], ret;
    data_size = recv(connection_socket, (char*)&tmp, MAX_LEN, 0);
    sscanf(tmp, "%c%s", &ret, job);
    creation_time = atof(job);
    finish_time = (double)(clock())/CLOCKS_PER_SEC;
    if (ret == 'f')
    {
        WaitForSingleObject(mutex, INFINITE);
        total_drops++;
        ReleaseMutex(mutex);
        fprintf(tmp_file, "%f %f %f\n", creation_time, 0, 0);
    }
    else{
        fprintf(tmp_file, "%f %f %f\n", creation_time, finish_time, (finish_time - creation_time));
    }
    WaitForSingleObject(mutex, INFINITE);
    total_threads--;
    ReleaseMutex(mutex);
    ExitThread(0);
}

double exp_wait_time(double mu)
{
    double u = (double)rand()/(double)RAND_MAX;
    return -1000*log(u)/mu;
}