#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 10

struct pasticceria_t
{
    sem_t cliente, commesso, full, empty;

} pasticceria;

void init_pasticceria(struct pasticceria_t *p)
{
    sem_init(&p->cliente, 0, 0);
    sem_init(&p->full, 0, 0);
    sem_init(&p->empty, 0, N);
    sem_init(&p->commesso, 0, 0);
}

void cuoco_inizio_torta(struct pasticceria_t *p)
{
    sem_wait(&p->empty);
}

void cuoco_fine_torta(struct pasticceria_t *p)
{
    sem_post(&p->full);
}

void commesso_prende_torta(struct pasticceria_t *p)
{
    sem_wait(&p->full);
    sem_post(&p->empty);
}

void commesso_vendo_torta(struct pasticceria_t *p)
{
    sem_post(&p->commesso);
    sem_wait(&p->cliente);

}

void cliente_acquisto(struct pasticceria_t *p)
{  
    sem_post(&p->cliente);
    sem_wait(&p->commesso);
}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

void *cuoco(void *args)
{
    while (1)
    {
        cuoco_inizio_torta(&pasticceria);
        cuoco_fine_torta(&pasticceria);
        printf("cuoco ha preparato una torta\n");
    }
}

void *commesso(void *args)
{
    while (1)
    {
        commesso_prende_torta(&pasticceria);
        printf("COMMESSO: vuole vendere una torta\n");
        commesso_vendo_torta(&pasticceria);
        pausetta();
    }
}

void *cliente(void *args)
{
    while (1)
    {
        cliente_acquisto(&pasticceria);
        printf("CLIENTE ACQUISTATO TORTA\n");
        pausetta();
    }
}

int main()
{
    // inizializzo thread e attr
    pthread_t Tcuoco;
    pthread_t Tcommesso;
    pthread_t Tcliente1;
    pthread_t Tcliente2;
    pthread_t Tcliente3;

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_pasticceria(&pasticceria);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    pthread_create(&Tcuoco, &attr, cuoco, NULL);
    pthread_create(&Tcommesso, &attr, commesso, NULL);
    pthread_create(&Tcliente1, &attr, cliente, NULL);
    pthread_create(&Tcliente2, &attr, cliente, NULL);
    pthread_create(&Tcliente3, &attr, cliente, NULL);
    pthread_attr_destroy(&attr);

    sleep(5);

    printf("Fine Main\n");
    return 0;
}