#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CYCLE 50
#define BUSY 1000000

sem_t mutex;
sem_t semA;
sem_t semB;
sem_t semR;

int c_R = 0;
int b_R = 0;
int c_A = 0;
int b_A = 0;
int b_B = 0;
int c_B = 0;

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

// Procedure di A
void StartProcA(void)
{
    sem_wait(&mutex);

    if (c_R || b_R || c_A) // controllo se ci sono R blocatti per darli priorità
    {
        b_A++;
    }
    else
    {
        c_A++;
        sem_post(&semA);
    }

    sem_post(&mutex);
    sem_wait(&semA);
}

void EndProcA(void)
{
    sem_wait(&mutex);

    c_A--;

    if (b_R && !c_A && !c_B)
    {
        b_R--;
        c_R++;
        sem_post(&semR);
    }
    else if (b_A)
    {
        b_A--;
        c_A++;
        sem_post(&semA);
    }

    sem_post(&mutex);
}

// Procedure di B
void StartProcB(void)
{
    sem_wait(&mutex);

    if (c_R || b_R || c_B) // controllo se ci sono R blocatti per darli priorità
    {
        b_B++;
    }
    else
    {
        c_B++;
        sem_post(&semB);
    }

    sem_post(&mutex);
    sem_wait(&semB);
}

void EndProcB(void)
{
    sem_wait(&mutex);

    c_B--;

    if (b_R && !c_A && !c_B)
    {
        b_R--;
        c_R++;
        sem_post(&semR);
    }
    else if (b_B)
    {
        b_B--;
        c_B++;
        sem_post(&semB);
    }

    sem_post(&mutex);
}

// Procedure reset
void StartReset(void)
{
    sem_wait(&mutex);

    if (c_A || c_B)
    {
        b_R++;
    }
    else
    {
        c_R++;
        sem_post(&semR);
    }

    sem_post(&mutex);
    sem_wait(&semR);
}

void EndReset(void)
{
    sem_wait(&mutex);

    c_R--;

    if (b_A)
    {
        c_A++;
        b_A--;
        sem_post(&semA);
    }

    if (b_B)
    {
        c_B++;
        b_B--;
        sem_post(&semB);
    }

    sem_post(&mutex);
}

void *esegueA1(void *args)
{

    for (;;)
    {

        StartProcA();
        myprint("A");
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
        myprint("a");
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
        myprint("B");
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
        myprint("b");
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
        myprint("$");
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
        myprint(".");
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
    pthread_attr_init(&attr);

    sem_init(&mutex, 0, 1);
    sem_init(&semA, 0, 0);
    sem_init(&semB, 0, 0);
    sem_init(&semR, 0, 0);

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