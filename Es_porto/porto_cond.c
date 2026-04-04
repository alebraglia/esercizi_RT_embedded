#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 20

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct porto_t
{
    pthread_mutex_t mutex;
    pthread_cond_t entrata, uscita, cond_posti;
    int b_e, b_u, transito, posti;

} porto;

void init_porto(struct porto_t *p)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&p->mutex, &m_attr);
    pthread_cond_init(&p->entrata, &c_attr);
    pthread_cond_init(&p->uscita, &c_attr);
    pthread_cond_init(&p->cond_posti, &c_attr);

    pthread_mutexattr_destroy(&m_attr);
    pthread_condattr_destroy(&c_attr);
    p->b_e = p->b_u = p->transito = 0;
    p->posti = 4;
}

void entrata_richiesta(struct porto_t *p)
{
    pthread_mutex_lock(&p->mutex);

    while (p->posti < 1)
    {
        pthread_cond_wait(&p->cond_posti, &p->mutex);
    }
    p->posti--;

    while (p->transito >= 2)
    {
        p->b_e++;
        pthread_cond_wait(&p->entrata, &p->mutex);
        p->b_e--;
    }
    p->transito++;
    pthread_mutex_unlock(&p->mutex);
}

void entrata_ok(struct porto_t *p)
{
    pthread_mutex_lock(&p->mutex);

    p->transito--;

    if (p->b_u)
    {
        pthread_cond_signal(&p->uscita);
    } else if (p->b_e)
    {
        pthread_cond_signal(&p->entrata);
    }
    pthread_mutex_unlock(&p->mutex);
}

void uscita_richiesta(struct porto_t *p)
{
    pthread_mutex_lock(&p->mutex);

    while (p->transito >= 2)
    {
        p->b_u++;
        pthread_cond_wait(&p->uscita, &p->mutex);
        p->b_u--;
    }

    p->transito++;
    p->posti++;
    pthread_cond_signal(&p->cond_posti);
    pthread_mutex_unlock(&p->mutex);
}

void uscita_ok(struct porto_t *p)
{
    entrata_ok(p);
}

void *barca(void *arg)
{
    pausetta();
    int numero = *(int *)arg;
    printf("BARCA %d ARRIVA AL PORTO\n", numero);
    entrata_richiesta(&porto);
    printf("BARCA %d ENTRANDO\n", numero);
    entrata_ok(&porto);
    printf("BARCA %d STAZIONATA\n", numero);

    pausetta();

    printf("BARCA %d VUOLE USCIRE DAL PORTO\n", numero);
    uscita_richiesta(&porto);
    printf("BARCA %d IN USCITA\n", numero);
    uscita_ok(&porto);
    printf("BARCA %d USCITA\n", numero);
}

int main()
{
    // inizializzo thread e attr
    pthread_t Tbarca[N];
    int id_barca[N];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_porto(&porto);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < N; i++)
    {
        id_barca[i] = i;
        pthread_create(&Tbarca[i], &attr, barca, &id_barca[i]);
    }

    pthread_attr_destroy(&attr);

    sleep(5);

    printf("Fine Main\n");
    return 0;
}