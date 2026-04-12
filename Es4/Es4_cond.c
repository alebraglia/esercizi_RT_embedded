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
    pthread_mutex_t mutex;
    pthread_cond_t partenza, mossa, fine;
    int giocatori_in_attesa, via, arbitro_fine;
    int mossa[2];
} Morra_cinese;

void init_morra_cinese(struct morra_cinese_t *m)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_cond_init(&m->partenza, &c_attr);
    pthread_cond_init(&m->mossa, &c_attr);
    pthread_cond_init(&m->fine, &c_attr);
    pthread_mutex_init(&m->mutex, &m_attr);
    m->giocatori_in_attesa = 0;
    m->via = 0;
    m->arbitro_fine = 0;
}

void arbitro_aspetta_giocatori(struct morra_cinese_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->giocatori_in_attesa != 2)
    {
        pthread_cond_wait(&m->partenza, &m->mutex);
    }
    m->arbitro_fine = 0;

    pthread_mutex_unlock(&m->mutex);
}

void via(struct morra_cinese_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->via = 1;
    m->giocatori_in_attesa = 0;
    pthread_cond_broadcast(&m->partenza);

    pthread_mutex_unlock(&m->mutex);
}

void arbitro_risultato(struct morra_cinese_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->giocatori_in_attesa != 2)
    {
        pthread_cond_wait(&m->mossa, &m->mutex);
    }

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
    pthread_mutex_unlock(&m->mutex);
    read(0, &c, 1);
    
    pthread_mutex_lock(&m->mutex);
    m->giocatori_in_attesa = 0;
    m->via = 0;

    m->arbitro_fine = 1;
    pthread_cond_broadcast(&m->fine);

    pthread_mutex_unlock(&m->mutex);
}

void giocatore_aspetta_via(struct morra_cinese_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->giocatori_in_attesa++;
    pthread_cond_signal(&m->partenza);

    while (!m->via)
    {
        pthread_cond_wait(&m->partenza, &m->mutex);
    }
    pthread_mutex_unlock(&m->mutex);
}

void giocatore_fa_mossa(struct morra_cinese_t *m, int numerogiocatore)
{
    pthread_mutex_lock(&m->mutex);

    int mossa = rand() % 3;
    m->giocatori_in_attesa++;

    m->mossa[numerogiocatore] = mossa;
    pthread_cond_signal(&m->mossa);

    printf("GIOCATORE %d: %s\n", numerogiocatore, nomi_mosse[mossa]);
    while (!m->arbitro_fine)
    {
        pthread_cond_wait(&m->fine, &m->mutex);
    }

    pthread_mutex_unlock(&m->mutex);
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
