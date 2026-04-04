#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

// numero buste della mailbox
#define N 3

// scrittori
#define M 8

// lettori
#define R 5

#define NESSUNO -1

typedef int T;

typedef struct sem_priv
{
    sem_t s;
    int c;

} sem_priv;

typedef struct gestore
{
    sem_t mutex;
    int free;

    sem_priv priv[3]; // semafori per le 3 priorità
    sem_priv riceventi;

    int next[N];

} gestore;

typedef struct busta
{
    T data;
    int full;
} busta;

typedef struct mailbox
{
    busta b[N];
    gestore g;
} mailbox;

void init_sem_priv(sem_priv *sem)
{
    sem_init(&sem->s, 0, 0);
    sem->c = 0;
}

void init_gestore(gestore *gestore)
{
    sem_init(&gestore->mutex, 0,1);
    gestore->free = 0; 

    for (size_t i = 0; i < 3; i++)
    {
        init_sem_priv(&gestore->priv[i]);
        gestore->next[i] = NESSUNO;
    }
    init_sem_priv(&gestore->riceventi);
    
}

void init_mailbox(mailbox *m)
{
    init_gestore(&m->g);
}

busta gestore_richiedi_busta(gestore *g, int prio)
{
    int miblocco = 0;
    busta bustavuota;

    sem_wait(&g->mutex);
    
    /* devo capire se posso accedere alla coda */
    switch (prio)
    {
    case 0:
        miblocco = g->priv[0].c;
        break;

    case 1:
        miblocco = g->priv[0].c || g->priv[1].c;
        break;

    case 2:
        miblocco = g->priv[0].c || g->priv[1].c || g->priv[2].c;
    }

    if (miblocco || g->free == 0)
    {
        g->priv[prio].c++;
        
    }
    
}

void send(mailbox *m, T msg, int priorita)
{

}


/* ------------------------------ */

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

// un contatore
int cont;

/* i thread */

void *mittente(void *arg)
{
    int p = (int)arg; // Priorita'
    T i;

    for (;;)
    {
        i = ++cont;
        fprintf(stderr, "S %4d%6d\n", p, i);
        send(&mailbox, cont, p);
        pausetta();
    }
    return 0;
}

void *ricevente(void *arg)
{
    T i;

    for (;;)
    {
        i = receive(&mailbox);
        fprintf(stderr, " R%10d\n", i);
        pausetta();
    }
    return 0;
}

/* la creazione dei thread */

int main()
{
    pthread_attr_t a;
    pthread_t p;
    int i;

    /* inizializzo il mio sistema */
    init_mailbox(&mailbox);

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (i = 0; i < M; i++)
        pthread_create(&p, &a, mittente, (void *)(rand() % 3));

    for (i = 0; i < R; i++)
        pthread_create(&p, &a, ricevente, NULL);

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(3);

    return 0;
}
