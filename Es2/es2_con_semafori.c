#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CYCLE 50
#define BUSY 1000000

sem_t mutex;
sem_t semAB;
sem_t semR;

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

// Procedure di A
void StartProcA(void)
{

    sem_wait(&mutex);

    if (c_R || b_R)     // controllo se ci sono R blocatti per darli priorità
    {
        b_AB++;
    }
    else
    {
        sem_post(&semAB);
        c_AB++;
    }

    sem_post(&mutex);
    sem_wait(&semAB);
}

void EndProcA(void)
{
    sem_wait(&mutex);

    c_AB--;

    if (b_R && !c_AB)
    {
        b_R--;
        c_R++;
        sem_post(&semR);
    }

    sem_post(&mutex);
}

// Procedure di B
void StartProcB(void)
{
    StartProcA();
}

void EndProcB(void)
{
    EndProcA();
}

//Procedure reset
void StartReset(void)
{
    sem_wait(&mutex);

    if (c_AB)
    {
        b_R++;
    }
    else
    {
        sem_post(&semR);
        c_R++;
    }

    sem_post(&mutex);
    sem_wait(&semR);
}

void EndReset(void)
{
    sem_wait(&mutex);

    c_R--;

    while (b_AB)
    {
        sem_post(&semAB);
        b_AB--;
        c_AB++;
    }

    sem_post(&mutex);
}

void *esegueA(void* args)
{

    for (;;)
    {
        fprintf(stderr, "A");
        StartProcA();
        myprint("-");
        EndProcA();
        fprintf(stderr, "a");
    }
    return 0;
}

void *esegueB(void* args)
{
    for (;;)
    {
        fprintf(stderr, "B");
        StartProcB();
        myprint("+");
        EndProcB();
        fprintf(stderr, "b");
    }
    return 0;
}

void *esegueR(void* args)
{
    for (;;)
    {
        fprintf(stderr, "R");
        StartReset();
        myprint(".");
        EndReset();
        fprintf(stderr, "r");
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
    pthread_attr_init(&attr);

    sem_init(&mutex, 0, 1);
    sem_init(&semAB, 0, 0);
    sem_init(&semR, 0, 0);

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