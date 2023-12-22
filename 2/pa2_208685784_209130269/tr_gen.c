#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    /*intialize arguments*/
    int seed, N, T, out_port;
    double p, cur_p;
    bool d = false;
    
    /*check for correct number of arguments, if wrong exit with -1*/
    if (argc != 5)
    {
        if (argc == 6 && strcmp(argv[5], "-d") == 0)
        {
            d = true;
        }
        else
        {
            perror("Invalid number of arguments");
            exit(-1);
        }
    }
    N = atoi(argv[1]);
    T = atoi(argv[2]);
    seed = atoi(argv[3]);
    p = atof(argv[4]);

    /*arguements format checking - if wrong, perror and exit with -1*/
    
    /*check for correct format of N*/
    if (N <= 0)
    {
        perror("Invalid N\n");
        exit(-1);
    }
    /*check for correct format of T*/
    if (T <= 0)
    {
        perror("Invalid T\n");
        exit(-1);
    }
    /*check for correct format of seed*/
    if ((seed == 0 && strcmp(argv[3], "0") != 0))
    {
        perror("Invalid seed\n");
        exit(-1);
    }
    /*check for correct format of p*/
    if ((p == 0 && strcmp(argv[4], "0") != 0) || p < 0 || p > 1)
    {
        perror("Invalid p\n");
        exit(-1);
    }
    /*init seed*/
    srand(seed);
    /*iterate over num of steps*/
    for (int t = 0; t < T; t++)
    {
        /*iterate over n*/
        for (int n = 0; n < N; n++)
        {
            cur_p = (double)rand()/(double)RAND_MAX;
            if (cur_p < p)
            {
                if (d) /*if d flag, use prob as mentioned*/
                {
                    cur_p = (double)rand()/(double)RAND_MAX;
                    if (cur_p < (2.0/3))
                    {
                        out_port = n;
                    }
                    else
                    {
                        out_port = (n + 1) % N;
                    }
                    
                }
                else /*if not d flag, use regular prob*/
                {
                    cur_p = (double)rand()/(double)RAND_MAX;
                    out_port = (int)(cur_p * N) % N;
                }
                printf("%d %d %d\n", t, n, out_port);
            }
        }
    }
    exit(0);
}
