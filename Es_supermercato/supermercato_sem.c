#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 3
#define C 20

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct supermercato_t
{
    sem_t cassa[N], finito[N], arrivo_cliente[N], mutex;
    int coda_cassa[N], oggetti_cassa[N];
} supermercato;

void init_supermercato(struct supermercato_t *s)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    for (size_t i = 0; i < N; i++)
    {
        sem_init(&s->cassa[i], 0, 0);
        sem_init(&s->finito[i], 0, 0);
        sem_init(&s->arrivo_cliente[i], 0, 0);
        sem_init(&s->mutex, 0, 1);
        s->coda_cassa[i] = 0;
        s->oggetti_cassa[i] = 0;
    }
}

void cliente_pagamento(struct supermercato_t *s, int cliente, int num_oggetti)
{
    sem_wait(&s->mutex);

    int cassa = 0;
    int min = s->oggetti_cassa[cassa];

    for (size_t i = 0; i < N; i++)
    {
        if (s->oggetti_cassa[i] < min)
        {
            min = s->oggetti_cassa[i];
            cassa = i;
        }
    }

    printf("CLIENTE %d SCEGLIE CASSA %d\n", cliente, cassa);
    s->coda_cassa[cassa]++;
    s->oggetti_cassa[cassa] += num_oggetti;

    sem_post(&s->arrivo_cliente[cassa]);

    sem_post(&s->mutex);
    sem_wait(&s->cassa[cassa]);

    printf("CLIENTE %d SERVITO IN CASSA %d con %d items\n", cliente, cassa, num_oggetti);

    sem_wait(&s->finito[cassa]);
    sem_wait(&s->mutex);

    s->oggetti_cassa[cassa] -= num_oggetti;

    printf("CLIENTE %d torno a casa\n", cliente);
    sem_post(&s->mutex);
}

void cassiere_servo_cliente(struct supermercato_t *s, int cassa)
{
    sem_wait(&s->mutex);

    printf("CASSA %d PROSSIMO cliente\n", cassa);

    sem_post(&s->cassa[cassa]);
    sem_post(&s->mutex);
    sem_wait(&s->arrivo_cliente[cassa]);
    sem_wait(&s->mutex);
    s->coda_cassa[cassa]--;

    sem_post(&s->mutex);
}

void cassiere_fine_cliente(struct supermercato_t *s, int cassa)
{
    sem_wait(&s->mutex);

    printf("CASSA %d FINITO cliente\n", cassa);
    sem_post(&s->finito[cassa]);

    sem_post(&s->mutex);
}

void *cliente(void *arg)
{
    int numero = *(int *)arg;

    printf("CLIENTE %d fa la spesa\n", numero);
    pausetta();
    cliente_pagamento(&supermercato, numero, rand() % 10);
    pausetta();
}

void *cassiere(void *arg)
{
    int numero = *(int *)arg;
    while (1)
    {
        cassiere_servo_cliente(&supermercato, numero);
        pausetta();
        cassiere_fine_cliente(&supermercato, numero);
        pausetta();
    }
}

int main()
{
    // inizializzo thread e attr
    pthread_t clienti[C];
    pthread_t cassa;
    int id_cliente[C];
    int id_cassa[N];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_supermercato(&supermercato);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    printf("creo i thread\n");

    for (int i = 0; i < N; i++)
    {
        id_cassa[i] = i;
        pthread_create(&cassa, &attr, cassiere, &id_cassa[i]);
    }

    for (int j = 0; j < C; j++)
    {
        id_cliente[j] = j;
        pthread_create(&clienti[j], &attr, cliente, &id_cliente[j]);
    }

    pthread_attr_destroy(&attr);

    sleep(2);

    printf("Fine Main\n");
    return 0;
}