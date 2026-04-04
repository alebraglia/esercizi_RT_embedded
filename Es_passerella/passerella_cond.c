#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 10

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct passerella_t
{
    pthread_mutex_t mutex;
    pthread_cond_t s_barca, guardiano;
    int c_p, barca;
} passerella;

void init_passerella(struct passerella_t *p)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_addr;

    pthread_mutex_init(&p->mutex, &m_attr);
    pthread_cond_init(&p->s_barca, &c_addr);
    pthread_cond_init(&p->guardiano, &c_addr);

    pthread_mutexattr_destroy(&m_attr);
    pthread_condattr_destroy(&c_addr);

    p->c_p = 0;
    p->barca = 1;
}

int pedone_entro_passerella(struct passerella_t *p, int hofretta)
{
    pthread_mutex_lock(&p->mutex);

    if (hofretta && p->barca)
    {
        pthread_mutex_unlock(&p->mutex);
        return 0;
    }

    while (p->barca)
    {
        printf("pedone si FERMA a causa barca\n");
        pthread_cond_wait(&p->s_barca, &p->mutex);
    }

    p->c_p++;
    pthread_mutex_unlock(&p->mutex);
    return 1;
}

void pedone_esco_passerella(struct passerella_t *p)
{
    pthread_mutex_lock(&p->mutex);

    p->c_p--;

    if (p->barca && p->c_p == 0)
    {
        pthread_cond_signal(&p->guardiano);
    }

    pthread_mutex_unlock(&p->mutex);
}

void guardiano_abbasso_passerella(struct passerella_t *p)
{
    pthread_mutex_lock(&p->mutex);
    p->barca = 0;

    pthread_cond_broadcast(&p->s_barca);

    pthread_mutex_unlock(&p->mutex);
}

void guardiano_alzo_passerella(struct passerella_t *p)
{
    pthread_mutex_lock(&p->mutex);

    p->barca = 1;
    while (p->c_p)
    {
        printf("GUARDIANO aspetta che le persone lascino la passerella\n");
        pthread_cond_wait(&p->guardiano, &p->mutex);
        printf("GUARDIANO usciti tutti\n");
    }

    pthread_mutex_unlock(&p->mutex);
}

void *persone(void *arg)
{
    pausetta();
    int numero = *(int *)arg;
    while (1)
    {
        pausetta();
        int fretta = rand() % 2;
        printf("pedone %d ARRIVATO con fretta %d\n", numero, fretta);
        if (pedone_entro_passerella(&passerella, fretta))
        {
            printf("pedone %d ATTRAVERSA la passerella\n", numero);
            pausetta();
            pedone_esco_passerella(&passerella);
            printf("pedone %d ESCE dalla passerella\n", numero);
        }
        else
            printf("pedone %d ha fretta e cambia strada\n", numero);
    }
}

void *guardiano(void *arg)
{
    while (1)
    {
        printf("PASSERELLA GIU\n");
        guardiano_abbasso_passerella(&passerella);
        pausetta();
        printf("ARRIVA BARCA\n");
        guardiano_alzo_passerella(&passerella);
        printf("PASSERELLA SU\n");
        pausetta();
        printf("BARCA PASSATA\n");
    }
}

int main()
{
    // inizializzo thread e attr
    pthread_t persona[N];
    pthread_t Tguardiano;
    int id_persona[N];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_passerella(&passerella);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < N; i++)
    {
        id_persona[i] = i;
        pthread_create(&persona[i], &attr, persone, &id_persona[i]);
    }
    pthread_create(&Tguardiano, &attr, guardiano, NULL);

    pthread_attr_destroy(&attr);

    sleep(2);

    printf("Fine Main\n");
    return 0;
}