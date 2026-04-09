#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define CARTA 0
#define SASSO 1
#define FORBICI 2

const char *nomi_mosse[3] = {"carta", "sasso", "forbici"};
char c;

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct morra_cinese_t
{
    sem_t giocatori, arbitro, via, mutex;
    int mosse;
    int mossa[2];
} Morra_cinese;

void init_morra_cinese(struct morra_cinese_t *m)
{
    sem_init(&m->arbitro, 0, 0);
    sem_init(&m->giocatori, 0, 0);
    sem_init(&m->via, 0, 0);
    sem_init(&m->mutex, 0, 1);
    m->mosse = 0;
}

void arbitro_aspetta_giocatori(struct morra_cinese_t *m)
{
    sem_wait(&m->giocatori);
    sem_wait(&m->giocatori);
}

void via(struct morra_cinese_t *m)
{
    sem_post(&m->via);
    sem_post(&m->via);
}

void arbitro_risultato(struct morra_cinese_t *m)
{
    sem_wait(&m->arbitro);

    sem_wait(&m->mutex);

    if (m->mossa[0] == m->mossa[1])
    {
        printf("GIUDICE: PAREGGIO\n");
    }
    else if (m->mossa[0] == 2 && m->mossa[1] == 0)
    {
        printf("GIUDICE: VINCE GIOCATORE 0\n");
    }
    else if (m->mossa[0] == 0 && m->mossa[1] == 2)
    {
        printf("GIUDICE: VINCE GIOCATORE 1\n");
    }
    else if (m->mossa[0] < m->mossa[1])
    {
        printf("GIUDICE: VINCE GIOCATORE 0\n");
    }
    else
        printf("GIUDICE: VINCE GIOCATORE 1\n");

    printf("-------------PREMI INVIO PER PROSSIMA PARTITA-----------------\n");
    sem_post(&m->mutex);
    read(0, &c, 1);
    
    sem_wait(&m->mutex);

    m->mosse = 0;

    sem_post(&m->mutex);
}

void giocatore_aspetta_via(struct morra_cinese_t *m)
{
    sem_post(&m->giocatori);

    sem_wait(&m->via);
}

void giocatore_fa_mossa(struct morra_cinese_t *m, int numerogiocatore)
{
    sem_wait(&m->mutex);

    int mossa = rand() % 3;
    m->mosse++;
    m->mossa[numerogiocatore] = mossa;
    printf("GIOCATORE %d: %s\n", numerogiocatore, nomi_mosse[mossa]);

    if (m->mosse == 2)
    {
        sem_post(&m->arbitro);
    }

    sem_post(&m->mutex);
}

void *giocatore(void *arg)
{
    while (1)
    {
        int numero = *(int *)arg;
        giocatore_aspetta_via(&Morra_cinese);
        giocatore_fa_mossa(&Morra_cinese, numero);
    }
}

void *arbitro(void *arg)
{
    while (1)
    {
        arbitro_aspetta_giocatori(&Morra_cinese);
        via(&Morra_cinese);
        arbitro_risultato(&Morra_cinese);
    }
}

int main()
{
    // inizializzo thread e attr

    pthread_t giocatori[2];
    pthread_t giudice;
    int id[2];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_morra_cinese(&Morra_cinese);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < 2; i++)
    {
        id[i] = i;
        pthread_create(&giocatori[i], &attr, giocatore, &id[i]);
    }

    pthread_create(&giudice, &attr, arbitro, NULL);
    pthread_attr_destroy(&attr);

    sleep(20);

    printf("Fine Main\n");
    return 0;
}