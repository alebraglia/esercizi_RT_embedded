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
    pthread_mutex_t mutex;
    pthread_cond_t sezioni[S];
    int libera[S];
    int macchine[N], num_macchine;
} rotonda;

void init_rotonda(struct rotonda_t *r)
{
    pthread_condattr_t c_attr;
    pthread_mutexattr_t m_attr;
    pthread_mutex_init(&r->mutex, &m_attr);
    for (size_t i = 0; i < S; i++)
    {
        pthread_cond_init(&r->sezioni[i], &c_attr);
        r->libera[i] = 1;
    }
    
    for (size_t i = 0; i < N; i++)
    {
        r->macchine[i] = FUORI;
    }
    r->num_macchine = 0;
    
    pthread_mutexattr_destroy(&m_attr);
    pthread_condattr_destroy(&c_attr);
}

void entra(struct rotonda_t *r, int num_auto, int sezione)
{
    pthread_mutex_lock(&r->mutex);
    while (!r->libera[sezione] || r->num_macchine <= S-1)
    {
        pthread_cond_wait(&r->sezioni[sezione], &r->mutex);
    }
    r->num_macchine++;
    r->libera[sezione] = 0;
    r->macchine[num_auto] = sezione;

    pthread_mutex_unlock(&r->mutex);
}

void esci(struct rotonda_t *r, int num_auto)
{
    pthread_mutex_lock(&r->mutex);

    r->num_macchine--;
    int sezione_attuale = r->macchine[num_auto];
    r->macchine[num_auto] = FUORI;
    printf("auto %d ESCE dalla rotonda\n", num_auto);
    r->libera[sezione_attuale] = 1;
    pthread_cond_broadcast(&r->sezioni[sezione_attuale]);
    
    pthread_mutex_unlock(&r->mutex);
}

int sonoarrivato(struct rotonda_t *r, int num_auto, int destinazione)
{
    pthread_mutex_lock(&r->mutex);

    if (r->macchine[num_auto] == destinazione)
    {
        printf("auto %d ARRIVATA a destinazione %d\n", num_auto, destinazione);
        pthread_mutex_unlock(&r->mutex);
        return 0;
    }

    int sezione_attuale = r->macchine[num_auto];
    while (!r->libera[((sezione_attuale + 1) % S)])
    {
        pthread_cond_wait(&r->sezioni[((sezione_attuale + 1) % S)], &r->mutex);
    }
    r->libera[((sezione_attuale + 1) % S)] = 0;

    r->libera[sezione_attuale] = 1;
    pthread_cond_signal(&r->sezioni[sezione_attuale]);

    r->macchine[num_auto] = (sezione_attuale + 1) % S;

    pthread_mutex_unlock(&r->mutex);
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
