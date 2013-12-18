#include <stdio.h>
#include <time.h>

#define true 1
#define false 0
#define size 8190

char flags[size+1];

char b[80];

int main()
{
    int i,prime,k,count,iter;
    clock_t t1 = clock(), t2;
    time_t  t3;
    struct tm *myt;
    char times[80];

    setvbuf(stdout,b,_IOLBF,sizeof(b));
    time(&t3);
    myt = localtime(&t3);
    strftime(times,sizeof(times),"%A %I:%M:%S %p",myt);
    printf("100 iterations\n""Appended string\n");
    printf("Started at - %s\n",times);

    for (iter = 1; iter <= 100; iter++)
    {   count = 0;
        for (i = 0; i <= size; i++) flags[i] = true;
        for (i = 0; i <= size; i++)
        {   if (flags[i])
            {   prime = i + i + 3;
                for (k = i+prime; k <= size; k += prime)
                    flags[k] = false;
                count++;
            }
        }
    }
    printf("\n%d primes.\n", count);

    t2 = clock();
    printf("Time = %d csecs\n", (t2 - t1));
}
