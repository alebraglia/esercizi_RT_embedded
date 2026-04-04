#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define FUORI -1
#define N 20
#define S 5

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct rotonda_t
{
    sem_t sezioni[S], mutex;
    int macchine[N];
} rotonda;

void init_rotonda(struct rotonda_t *r)
{
    for (size_t i = 0; i < S; i++)
    {
        sem_init(&r->mutex, 0, 1);
        sem_init(&r->sezioni[i], 0, 1);
    }
    for (size_t i = 0; i < N; i++)
    {
        r->macchine[i] = FUORI;
    }
}

void entra(struct rotonda_t *r, int num_auto, int sezione)
{
    sem_wait(&r->sezioni[sezione]);
    sem_wait(&r->mutex);
    r->macchine[num_auto] = sezione;

    sem_post(&r->mutex);
}

void esci(struct rotonda_t *r, int num_auto)
{
    sem_wait(&r->mutex);
    int sezione_attuale = r->macchine[num_auto];
    r->macchine[num_auto] = FUORI;
    printf("auto %d ESCE dalla rotonda\n", num_auto);
    sem_post(&r->sezioni[sezione_attuale]);
    sem_post(&r->mutex);
}

int sonoarrivato(struct rotonda_t *r, int num_auto, int destinazione)
{
    sem_wait(&r->mutex);

    if (r->macchine[num_auto] == destinazione)
    {
        printf("auto %d ARRIVATA a destinazione %d\n", num_auto, destinazione);
        sem_post(&r->mutex);
        return 0;
    }

    int sezione_attuale = r->macchine[num_auto];
    sem_post(&r->mutex);

    sem_wait(&r->sezioni[(sezione_attuale + 1) % S]);
    sem_post(&r->sezioni[sezione_attuale]);
    sem_wait(&r->mutex);

    r->macchine[num_auto] = (sezione_attuale + 1) % S;

    sem_post(&r->mutex);
    return 1;
}

void *macchina(void *arg)
{
    pausetta();
    int numero = *(int *)arg;
    int ingresso = rand() % S;
    int destinazione = rand() % S;
    entra(&rotonda, numero, ingresso);
    printf("auto %d ENTRA nella sezione %d con destinazione %d\n", numero, ingresso, destinazione);
    do
    {
        printf("auto %d PERCORRE una sezione\n", numero);
    } while (sonoarrivato(&rotonda, numero, destinazione));
    esci(&rotonda, numero);
}

int main()
{
    // inizializzo thread e attr
    pthread_t Macchine[N];
    int id_macchina[N];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_rotonda(&rotonda);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < N; i++)
    {
        id_macchina[i] = i;
        pthread_create(&Macchine[i], &attr, macchina, &id_macchina[i]);
    }

    pthread_attr_destroy(&attr);

    sleep(5);

    printf("Fine Main\n");
    return 0;
}