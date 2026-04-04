#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_cond_t condA;
pthread_cond_t condB;
pthread_cond_t condR;

#define CYCLE 50
#define BUSY 1000000

#define ANSI_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_CYAN "\033[36m"

int c_R = 0;
int b_R = 0;
int c_A = 0;
int b_A = 0;
int c_B = 0;
int b_B = 0;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

void myprint(const char *s)
{
    int i, j;
    fprintf(stderr, "[");
    for (j = 0; j < CYCLE; j++)
    {
        fprintf(stderr, "%s", s);
        for (i = 0; i < BUSY; i++)
            ;
    }
    fprintf(stderr, "]");
}

void StartProcA()
{
    pthread_mutex_lock(&mutex);

    while (b_R || c_R || c_A)
    {
        b_A++;
        pthread_cond_wait(&condA, &mutex);
        b_A--;
    }

    c_A++;
    pthread_mutex_unlock(&mutex);
}

void EndProcA(void)
{
    pthread_mutex_lock(&mutex);
    c_A--;

    if (b_R && !c_A && !c_B)
    {
        b_R--;
        pthread_cond_signal(&condR);
    }
    else if (b_A)
    {
        pthread_cond_signal(&condA);
    }

    pthread_mutex_unlock(&mutex);
}

void StartProcB()
{
    pthread_mutex_lock(&mutex);

    while (b_R || c_R || c_B)
    {
        b_B++;
        pthread_cond_wait(&condB, &mutex);
        b_B--;
    }

    c_B++;
    pthread_mutex_unlock(&mutex);
}

void EndProcB()
{
    pthread_mutex_lock(&mutex);
    c_B--;

    if (b_R && !c_A && !c_B)
    {
        b_R--;
        pthread_cond_signal(&condR);
    }
    else if (b_B)
    {
        pthread_cond_signal(&condB);
    }

    pthread_mutex_unlock(&mutex);
}

void StartReset()
{
    pthread_mutex_lock(&mutex);

    while (c_A || c_B)
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

    if (b_A)
    {
        pthread_cond_broadcast(&condA);
    }

    if (b_B)
    {
        pthread_cond_broadcast(&condB);
    }

    pthread_mutex_unlock(&mutex);
}

void *esegueA1(void *args)
{

    for (;;)
    {

        StartProcA();
        myprint(ANSI_RED ANSI_BOLD "A" ANSI_RESET);
        EndProcA();
        pausetta();
    }
    return 0;
}

void *esegueA2(void *args)
{

    for (;;)
    {

        StartProcA();
        myprint(ANSI_YELLOW "a" ANSI_RESET);
        EndProcA();
        pausetta();
    }
    return 0;
}

void *esegueB1(void *args)
{
    for (;;)
    {

        StartProcB();
        myprint(ANSI_BLUE ANSI_BOLD "B" ANSI_RESET);
        EndProcB();
        pausetta();
    }
    return 0;
}

void *esegueB2(void *args)
{
    for (;;)
    {

        StartProcB();
        myprint(ANSI_CYAN "b" ANSI_RESET);
        EndProcB();
        pausetta();
    }
    return 0;
}

void *esegueB3(void *args)
{
    for (;;)
    {
        StartProcB();
        myprint(ANSI_GREEN "$" ANSI_RESET);
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
        myprint(ANSI_BOLD "." ANSI_RESET);
        EndReset();
        pausetta();
    }
    return 0;
}

int main()
{
    // inizializzo thread e attr
    pthread_t proc_A1;
    pthread_t proc_A2;
    pthread_t proc_B1;
    pthread_t proc_B2;
    pthread_t proc_B3;
    pthread_t proc_R;

    pthread_attr_t attr;

    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;
    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_attr_init(&attr);
    pthread_mutex_init(&mutex, &m_attr);
    pthread_cond_init(&condA, &c_attr);
    pthread_cond_init(&condB, &c_attr);
    pthread_cond_init(&condR, &c_attr);

    srand(555);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    pthread_create(&proc_A1, &attr, (void *)esegueA1, NULL);
    pthread_create(&proc_A2, &attr, (void *)esegueA2, NULL);
    pthread_create(&proc_B1, &attr, (void *)esegueB1, NULL);
    pthread_create(&proc_B2, &attr, (void *)esegueB2, NULL);
    pthread_create(&proc_B3, &attr, (void *)esegueB3, NULL);
    pthread_create(&proc_R, &attr, (void *)esegueR, NULL);
    pthread_attr_destroy(&attr);

    sleep(5);

    printf("Fine Main\n");
    return 0;
}