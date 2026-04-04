#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 20

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct porto_t
{
    sem_t entrata, uscita, posti, mutex;
    int b_e, b_u, transito;

} porto;

void init_porto(struct porto_t *p)
{
    sem_init(&p->entrata, 0, 0);
    sem_init(&p->uscita, 0, 0);
    sem_init(&p->posti, 0, 4);
    sem_init(&p->mutex, 0, 1);
    p->b_e = p->b_u = p->transito = 0;
}

void entrata_richiesta(struct porto_t *p)
{
    sem_wait(&p->posti);
    sem_wait(&p->mutex);

    if (p->transito < 2)
    {
        p->transito++;
        sem_post(&p->entrata);
    }
    else
        p->b_e++;

    sem_post(&p->mutex);
    sem_wait(&p->entrata);
}

void entrata_ok(struct porto_t *p)
{
    sem_wait(&p->mutex);
    p->transito--;
    if (p->b_u)
    {
        p->b_u--;
        sem_post(&p->uscita);
    }
    else if (p->b_e)
    {
        p->b_e--;
        sem_post(&p->entrata);
    }

    sem_post(&p->mutex);
}

void uscita_richiesta(struct porto_t *p)
{
    sem_wait(&p->mutex);

    if (p->transito < 2)
    {
        p->transito++;
        sem_post(&p->uscita);
    }
    else
        p->b_u++;

    sem_post(&p->mutex);
    sem_wait(&p->uscita);
}

void uscita_ok(struct porto_t *p)
{
    entrata_ok(p);
    sem_post(&p->posti);
}

void *barca(void *arg)
{
    pausetta();
    int numero = *(int *)arg;
    printf("BARCA %d ARRIVA AL PORTO\n", numero);
    entrata_richiesta(&porto);
    printf("BARCA %d ENTRANDO\n", numero);
    entrata_ok(&porto);
    printf("BARCA %d STAZIONATA\n", numero);

    pausetta();

    printf("BARCA %d VUOLE USCIRE DAL PORTO\n", numero);
    uscita_richiesta(&porto);
    printf("BARCA %d IN USCITA\n", numero);
    uscita_ok(&porto);
    printf("BARCA %d USCITA\n", numero);
}

int main()
{
    // inizializzo thread e attr
    pthread_t Tbarca[N];
    int id_barca[N];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_porto(&porto);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < N; i++)
    {
        id_barca[i] = i;
        pthread_create(&Tbarca[i], &attr, barca, &id_barca[i]);
    }

    pthread_attr_destroy(&attr);

    sleep(5);

    printf("Fine Main\n");
    return 0;
}