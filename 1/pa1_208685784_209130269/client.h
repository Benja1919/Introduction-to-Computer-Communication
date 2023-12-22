#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include <math.h>
#include <time.h>
#include <Windows.h>
#pragma comment(lib,"ws2_32.lib")

#define MAX_LEN 4096

double exp_wait_time(double mu);
DWORD WINAPI job_creator(void* data);