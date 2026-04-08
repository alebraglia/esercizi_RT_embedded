#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 3

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct bagno_t
{
    pthread_mutex_t mutex;
    pthread_cond_t uomini, donne, seconda_donna;
    int seq_donne, occupato, b_m, b_d, aspetta, seconda_entra;
} bagno;

void init_bagno(struct bagno_t *b)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_cond_init(&b->uomini, &c_attr);
    pthread_cond_init(&b->donne, &c_attr);
    pthread_cond_init(&b->seconda_donna, &c_attr);
    pthread_mutex_init(&b->mutex, &m_attr);
    b->b_d = 0;
    b->b_m = 0;
    b->occupato = 0;
    b->seq_donne = 0;
    b->aspetta = 0;
    b->seconda_entra = 0;
}

void uomo_entra(struct bagno_t *b)
{
    pthread_mutex_lock(&b->mutex);

    while (b->occupato)
    {
        b->b_m++;
        pthread_cond_wait(&b->uomini, &b->mutex);
        b->b_m--;
    }
    b->occupato = 1;

    pthread_mutex_unlock(&b->mutex);
}

void uomo_esci(struct bagno_t *b)
{
    pthread_mutex_lock(&b->mutex);

    b->occupato = 0;
    if (b->b_d)
    {
        if (b->b_d > 1)
        {
            b->aspetta = 1;
            b->seconda_entra = 0;
            printf("DONNE entrano in COPPIA\n");
            pthread_cond_signal(&b->donne);
        }
        pthread_cond_signal(&b->donne);
    }
    else if (b->b_m)
    {
        pthread_cond_signal(&b->uomini);
    }

    pthread_mutex_unlock(&b->mutex);
}

void donna_entra(struct bagno_t *b)
{
    pthread_mutex_lock(&b->mutex);

    while (b->occupato)
    {
        b->b_d++;
        pthread_cond_wait(&b->donne, &b->mutex);
        b->b_d--;
    }

    pthread_cond_signal(&b->seconda_donna);
    while (b->aspetta && b->seconda_entra == 0)
    {
        b->seconda_entra = 1;
        pthread_cond_wait(&b->seconda_donna, &b->mutex);
    }

    b->occupato = 1;
    b->seq_donne++;
    pthread_mutex_unlock(&b->mutex);
}

void donna_esce(struct bagno_t *b)
{
    pthread_mutex_lock(&b->mutex);

    b->occupato = 0;
    if (b->aspetta != 1)
    {
        if (b->b_d && (b->seq_donne < N || b->b_m == 0))
        {
            if (b->b_d > 1)
            {
                b->aspetta = 1;
                b->seconda_entra = 0;
                printf("DONNE entrano in COPPIA\n");
                pthread_cond_signal(&b->donne);
            }
            pthread_cond_signal(&b->donne);
        }
        else if (b->b_m)
        {
            b->seq_donne = 0;
            pthread_cond_signal(&b->uomini);
        }
    }
    else
    {
        b->aspetta = 0;
    }

    pthread_mutex_unlock(&b->mutex);
}

void *uomo(void *arg)
{
    pausetta();
    printf("UOMO vuole ENTRARE\n");
    uomo_entra(&bagno);
    printf("UOMO ENTRA\n");
    pausetta();
    printf("UOMO ESCE\n");
    uomo_esci(&bagno);
}

void *donna(void *arg)
{
    pausetta();
    printf("DONNA vuole ENTRARE\n");
    donna_entra(&bagno);
    printf("DONNA ENTRA\n");
    pausetta();
    printf("DONNA ESCE\n");
    donna_esce(&bagno);
}

int main()
{
    // inizializzo thread e attr

    pthread_t donne[15];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_bagno(&bagno);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < 15; i++)
    {
        pthread_create(&donne[i], &attr, donna, NULL);
    }

    pthread_attr_destroy(&attr);

    sleep(2);

    printf("Fine Main\n");
    return 0;
}