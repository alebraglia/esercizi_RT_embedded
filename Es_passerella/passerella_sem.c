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
    sem_t s_barca, guardiano, mutex;
    int b_p, c_p, barca;
} passerella;

void init_passerella(struct passerella_t *p)
{
    sem_init(&p->s_barca, 0, 0);
    sem_init(&p->guardiano, 0, 0);
    sem_init(&p->mutex, 0, 1);
    p->b_p = 0;
    p->c_p = 0;
    p->barca = 1;
}

int pedone_entro_passerella(struct passerella_t *p, int hofretta)
{
    sem_wait(&p->mutex);

    if (p->barca)
    {
        if (hofretta)
        {
            sem_post(&p->mutex);
            return 0;
        }

        p->b_p++;
        sem_post(&p->mutex);
        printf("pedone si FERMA a causa barca\n");
        sem_wait(&p->s_barca);
        sem_wait(&p->mutex);
    }

    p->c_p++;
    sem_post(&p->mutex);
    return 1;
}

void pedone_esco_passerella(struct passerella_t *p)
{
    sem_wait(&p->mutex);

    p->c_p--;

    if (p->barca && p->c_p == 0)
    {
        sem_post(&p->guardiano);
    }

    sem_post(&p->mutex);
}

void guardiano_abbasso_passerella(struct passerella_t *p)
{
    sem_wait(&p->mutex);
    p->barca = 0;
    while (p->b_p)
    {
        p->b_p--;
        sem_post(&p->s_barca);
    }

    sem_post(&p->mutex);
}

void guardiano_alzo_passerella(struct passerella_t *p)
{
    sem_wait(&p->mutex);

    p->barca = 1;
    if (p->c_p)
    {
        sem_post(&p->mutex);
        printf("GUARDIANO aspetta che le persone lascino la passerella\n");
        sem_wait(&p->guardiano);
        printf("GUARDIANO usciti tutti\n");
        sem_wait(&p->mutex);
    }

    sem_post(&p->mutex);
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