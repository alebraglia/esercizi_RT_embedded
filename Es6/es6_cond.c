#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_FILOSOFI 5
#define DRAFT 1

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct tavolo_t
{
    pthread_mutex_t mutex;
    pthread_cond_t posata[NUM_FILOSOFI];

    int libera[NUM_FILOSOFI];
} tavolo;

void init_tavolo(struct tavolo_t *t)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    for (size_t i = 0; i < NUM_FILOSOFI; i++)
    {
        pthread_cond_init(&t->posata[i], &c_attr);
        t->libera[i] = 1;
    }

    pthread_mutex_init(&t->mutex, &m_attr);
}

void pensa(struct tavolo_t *t, int filosofo)
{
    pthread_mutex_lock(&t->mutex);

    t->libera[(filosofo + 1) % NUM_FILOSOFI] = 1;
    pthread_cond_signal(&t->posata[(filosofo + 1) % NUM_FILOSOFI]);

    t->libera[filosofo] = 1;
    pthread_cond_signal(&t->posata[filosofo]);

    pthread_mutex_unlock(&t->mutex);
    pausetta();
    sleep(DRAFT);
}

void mangia(struct tavolo_t *t, int filosofo)
{
    pthread_mutex_lock(&t->mutex);

    while (!t->libera[(filosofo + 1) % NUM_FILOSOFI])
    {
        pthread_cond_wait(&t->posata[(filosofo + 1) % NUM_FILOSOFI], &t->mutex);
    }
    printf("FILOSOFO %d ACQUISISCE POSATA %d\n", filosofo, (filosofo + 1) % NUM_FILOSOFI);
    t->libera[(filosofo + 1) % NUM_FILOSOFI] = 0;

    while (!t->libera[filosofo])
    {
        pthread_cond_wait(&t->posata[filosofo], &t->mutex);
    }
    printf("FILOSOFO %d ACQUISISCE POSATA %d\n", filosofo, filosofo);
    t->libera[filosofo] = 0;

    printf("FILOSOFO %d STA MANGIANDO\n", filosofo);
    pthread_mutex_unlock(&t->mutex);
}

void *filosofo(void *arg)
{
    int numero = *(int *)arg;
    while (1)
    {

        printf("FILOSOFO %d PENSA\n", numero);
        pensa(&tavolo, numero);

        printf("FILOSOFO %d VUOLE MANGIARE\n", numero);
        mangia(&tavolo, numero);
        pausetta();
    }
}


int main()
{
    // inizializzo thread e attr

    pthread_t filosofi[NUM_FILOSOFI];
    int id[NUM_FILOSOFI];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_tavolo(&tavolo);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < NUM_FILOSOFI; i++)
    {
        id[i] = i;
        pthread_create(&filosofi[i], &attr, filosofo, &id[i]);
    }

    pthread_attr_destroy(&attr);

    sleep(20);

    printf("Fine Main\n");
    return 0;
}