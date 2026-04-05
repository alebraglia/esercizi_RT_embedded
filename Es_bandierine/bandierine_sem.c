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
    sem_t partenza, giudice, fine, mutex;
    int vincitore, giocatori_arrivati, salvo;
} bandierine;

void init_bandierine(struct bandierine_t *b)
{
    sem_init(&b->mutex, 0, 1);
    sem_init(&b->fine, 0, 0);
    sem_init(&b->giudice, 0, 0);
    sem_init(&b->partenza, 0, 0);
    b->vincitore = -1;
    b->giocatori_arrivati = 0;
    b->salvo = 0;
}

void attendi_il_via(struct bandierine_t *b)
{
    sem_wait(&b->mutex);
    b->giocatori_arrivati++;

    if (b->giocatori_arrivati == 2)
    {
        sem_post(&b->giudice);
    }

    sem_post(&b->mutex);
    sem_wait(&b->partenza);
}

int bandierina_presa(struct bandierine_t *b, int numerogiocatore)
{
    sem_wait(&b->mutex);

    if (b->vincitore == -1)
    {
        b->vincitore = numerogiocatore;
        sem_post(&b->mutex);
        return 1;
    }

    sem_post(&b->mutex);
    return 0;
}

int sono_salvo(struct bandierine_t *b, int numerogiocatore)
{
    sem_wait(&b->mutex);

    if (b->vincitore == numerogiocatore)
    {
        b->salvo = 1;
        b->giocatori_arrivati++;
        sem_post(&b->mutex);
        sem_post(&b->fine);
        return 1;
    }

    sem_post(&b->mutex);
    return 0;
}

int ti_ho_preso(struct bandierine_t *b, int numerogiocatore)
{
    sem_wait(&b->mutex);

    if (b->salvo == 0)
    {
        b->vincitore = numerogiocatore;
        b->giocatori_arrivati++;
        sem_post(&b->mutex);
        sem_post(&b->fine);
        return 1;
    }

    sem_wait(&b->mutex);
    return 0;
}

void attendi_giocatori(struct bandierine_t *b)
{
    sem_wait(&b->mutex);

    if (b->giocatori_arrivati != 2)
    {
        sem_post(&b->mutex);
        sem_wait(&b->giudice);
        sem_wait(&b->mutex);
    }
    b->giocatori_arrivati = 0;

    sem_post(&b->mutex);
}

void via(struct bandierine_t *b)
{
    sem_post(&b->partenza);
    sem_post(&b->partenza);
}

int risultato_gioco(struct bandierine_t *b)
{
    sem_wait(&b->fine);
    return b->vincitore;
}

void *giocatore(void *arg)
{
    int numero_giocatore = *(int *)arg;
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