#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 2

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct bandierine_t
{
    pthread_mutex_t mutex;
    pthread_cond_t partenza, giudice, fine;
    int vincitore, giocatori_arrivati, salvo;
} bandierine;

void init_bandierine(struct bandierine_t *b)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;
    pthread_mutex_init(&b->mutex, &m_attr);
    pthread_cond_init(&b->partenza, &c_attr);
    pthread_cond_init(&b->giudice, &c_attr);
    pthread_cond_init(&b->fine, &c_attr);

    pthread_mutexattr_destroy(&m_attr);
    pthread_condattr_destroy(&c_attr);
    b->vincitore = -1;
    b->giocatori_arrivati = 0;
    b->salvo = 0;
}

void attendi_il_via(struct bandierine_t *b)
{
    pthread_mutex_lock(&b->mutex);
    b->giocatori_arrivati++;
    if (b->giocatori_arrivati == 2)
    {
        pthread_cond_signal(&b->giudice);
    }

    pthread_cond_wait(&b->partenza, &b->mutex);
    pthread_mutex_unlock(&b->mutex);
}

int bandierina_presa(struct bandierine_t *b, int numerogiocatore)
{
    pthread_mutex_lock(&b->mutex);

    if (b->vincitore == -1)
    {
        b->vincitore = numerogiocatore;
        pthread_mutex_unlock(&b->mutex);
        return 1;
    }

    pthread_mutex_unlock(&b->mutex);
    return 0;
}

int sono_salvo(struct bandierine_t *b, int numerogiocatore)
{
    pthread_mutex_lock(&b->mutex);

    if (b->vincitore == numerogiocatore)
    {
        b->salvo = 1;
        b->giocatori_arrivati++;
        pthread_mutex_unlock(&b->mutex);
        pthread_cond_signal(&b->fine);
        return 1;
    }

    pthread_mutex_unlock(&b->mutex);
    return 0;
}

int ti_ho_preso(struct bandierine_t *b, int numerogiocatore)
{
    pthread_mutex_lock(&b->mutex);

    if (b->salvo == 0)
    {
        b->vincitore = numerogiocatore;
        b->giocatori_arrivati++;
        pthread_mutex_unlock(&b->mutex);
        pthread_cond_signal(&b->fine);
        return 1;
    }

    pthread_mutex_unlock(&b->mutex);
    return 0;
}

void attendi_giocatori(struct bandierine_t *b)
{
    pthread_mutex_lock(&b->mutex);

    if (b->giocatori_arrivati != 2)
    {
        pthread_cond_wait(&b->giudice, &b->mutex);
    }
    b->giocatori_arrivati = 0;

    pthread_mutex_unlock(&b->mutex);
}

void via(struct bandierine_t *b)
{
    pthread_cond_broadcast(&b->partenza);
}

int risultato_gioco(struct bandierine_t *b)
{
    pthread_mutex_lock(&b->mutex);

    if (b->giocatori_arrivati == 0)
    {
        pthread_cond_wait(&b->fine, &b->mutex);
    }

    pthread_mutex_unlock(&b->mutex);
    return b->vincitore;
}

void *giocatore(void *arg)
{
    int numero_giocatore = *(int*)arg;
    attendi_il_via(&bandierine);
    printf("%d INIZIA a correre\n", numero_giocatore);
    pausetta();
    if (bandierina_presa(&bandierine, numero_giocatore))
    {
        printf("%d PRENDE BANDIERA\n", numero_giocatore);
        pausetta();
        if (sono_salvo(&bandierine, numero_giocatore))
            printf("%d SALVO\n", numero_giocatore);
    }
    else
    {
        printf("%d NON PRENDE BANDIERA\n", numero_giocatore);
        pausetta();
        if (ti_ho_preso(&bandierine, numero_giocatore))
            printf("%d PRESO GIOCATORE\n", numero_giocatore);
    }
    return 0;
}

void *giudice(void *arg)
{
    attendi_giocatori(&bandierine);
    printf("GIUDICE PRONTI?\n");
    pausetta();
    printf("GIUDICE VIA!\n");
    via(&bandierine);
    printf("VINCITORE E': %d\n", risultato_gioco(&bandierine));
    return 0;
}

int main()
{
    // inizializzo thread e attr
    pthread_t giocatori[N];
    pthread_t Tgiudice;
    int id_giocatore[N];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_bandierine(&bandierine);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < N; i++)
    {
        id_giocatore[i] = i;
        pthread_create(&giocatori[i], &attr, giocatore, &id_giocatore[i]);
    }
    pthread_create(&Tgiudice, &attr, giudice, NULL);

    pthread_attr_destroy(&attr);

    sleep(2);

    printf("Fine Main\n");
    return 0;
}