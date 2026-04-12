#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 7

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct urna_t
{
    pthread_mutex_t mutex;
    pthread_cond_t attende_risultato;
    int risultato, voti_1, voti_0;
} u;

void vota (int voto)
{
    pthread_mutex_lock(&u.mutex);

    if (voto)
    {
        u.voti_1++;
    }
    else u.voti_0++;
    

    pthread_mutex_unlock(&u.mutex);
}

int risultato()
{
    pthread_mutex_lock(&u.mutex);

    if (u.voti_0 > N/2 || u.voti_1 > N/2)
    {
        u.risultato = 1;        
        pthread_cond_broadcast(&u.attende_risultato);
    }

    while (!u.risultato)
    {
        pthread_cond_wait(&u.attende_risultato, &u.mutex);
    }

    pthread_mutex_unlock(&u.mutex);
    return u.voti_1 > u.voti_0;
}

void *thread(void* arg)
{
    int voto = rand() % 2;
    vota(voto);

    printf("thread VOTA %d\n", voto);
    if (voto == risultato())
    {
        printf("HO VINTO\n");
    }
    else printf("HO PERSO\n");
    
}

void init_urna()
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_cond_init(&u.attende_risultato, &c_attr);
    
    pthread_mutex_init(&u.mutex, &m_attr);
    u.voti_0 = 0;
    u.voti_1 = 0;
    u.risultato = 0;
}

int main()
{
    // inizializzo thread e attr

    pthread_t giocatori[N];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(212);
    init_urna();
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < N; i++)
    {
        pthread_create(&giocatori[i], &attr, thread, NULL);
    }

    pthread_attr_destroy(&attr);

    sleep(1);

    printf("Fine Main\n");
    return 0;
}