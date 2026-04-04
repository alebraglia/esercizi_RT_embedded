#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_cond_t condAB;
pthread_cond_t condR;

#define CYCLE 50
#define BUSY 1000000

int c_R = 0;
int b_R = 0;
int c_AB = 0;
int b_AB = 0;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

void myprint(char *s)
{
    int i, j;
    fprintf(stderr, "[");
    for (j = 0; j < CYCLE; j++)
    {
        fprintf(stderr, s);
        for (i = 0; i < BUSY; i++)
            ;
    }
    fprintf(stderr, "]");
}

void StartProcA()
{
    pthread_mutex_lock(&mutex);

    while (b_R || c_R)
    {
        b_AB++;
        pthread_cond_wait(&condAB, &mutex);
        b_AB--;
    }

    c_AB++;
    pthread_mutex_unlock(&mutex);
}

void EndProcA(void)
{
    pthread_mutex_lock(&mutex);
    c_AB--;

    if (b_R && !c_AB)
    {
        b_R--;
        pthread_cond_signal(&condR);
    }

    pthread_mutex_unlock(&mutex);
}

void StartProcB() { StartProcA(); }
void EndProcB() { EndProcA(); }

void StartReset()
{
    pthread_mutex_lock(&mutex);

    while (c_AB)
    {
        b_R++;
        pthread_cond_wait(&condR, &mutex);
    }
    
    c_R++;
    pthread_mutex_unlock(&mutex);
}

void EndReset()
{
    pthread_mutex_lock(&mutex);
    c_R--;

    if (b_AB)
    {
        pthread_cond_broadcast(&condAB);
    }
    pthread_mutex_unlock(&mutex);
}

void *esegueA(void *args)
{
    for (;;)
    {
        StartProcA();
        myprint("a");
        EndProcA();
        pausetta();
    }
    return 0;
}

void *esegueB(void *args)
{
    for (;;)
    {
        StartProcB();
        myprint("b");
        EndProcB();
        pausetta();
    }
    return 0;
}

void *esegueR(void *args)
{
    for (;;)
    {
        StartReset();
        myprint("r");
        EndReset();
        pausetta();
    }
    return 0;
}

int main()
{
    // inizializzo thread e attr
    pthread_t proc_A;
    pthread_t proc_B;
    pthread_t proc_R;

    pthread_attr_t attr;

    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;
    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_attr_init(&attr);
    pthread_mutex_init(&mutex, &m_attr);
    pthread_cond_init(&condAB, &c_attr);
    pthread_cond_init(&condR, &c_attr);

    srand(555);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    pthread_create(&proc_A, &attr, (void *)esegueA, NULL);
    pthread_create(&proc_B, &attr, (void *)esegueB, NULL);
    pthread_create(&proc_R, &attr, (void *)esegueR, NULL);
    pthread_attr_destroy(&attr);

    sleep(5);

    printf("Fine Main\n");
    return 0;
}