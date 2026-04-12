#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 10
#define SHAVING_ITERATIONS 3
#define PAYING_ITERATIONS 3

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct barbiere_t
{
    sem_t fuori, cassa, barbiere;
} barbiere;

void init_barbiere(struct barbiere_t *b)
{
    sem_init(&b->barbiere, 0, 3);
    sem_init(&b->cassa, 0, 1);
    sem_init(&b->fuori, 0, 4);

}

void cliente_arriva(struct barbiere_t *b)
{
    sem_wait(&b->fuori);
}

void cliente_entra(struct barbiere_t *b, int cliente)
{
    printf("Cliente %d ENTRATO\n", cliente);

    sem_wait(&b->barbiere);
    sem_post(&b->fuori);

    printf("Cliente %d STA TAGLIANDO BARBA\n", cliente);
    for (size_t i = 0; i < SHAVING_ITERATIONS; i++)
    {
        pausetta();
    }

    printf("Cliente %d FINITO TAGLIO\n", cliente);

    sem_post(&b->barbiere);
}

void cliente_paga(struct barbiere_t *b, int cliente)
{

    printf("Cliente %d ASPETTA CASSA\n", cliente);
    sem_wait(&b->cassa);

    printf("Cliente %d PAGA\n", cliente);
    for (size_t i = 0; i < PAYING_ITERATIONS; i++)
    {
        pausetta();
    }

    printf("Cliente %d ESCE\n", cliente);

    sem_post(&b->cassa);
}

void *cliente(void *arg)
{
    int numero = *(int *)arg;
    while (1)
    {
        pausetta();
        printf("Cliente %d ARRIVA\n", numero);
        cliente_arriva(&barbiere);

        cliente_entra(&barbiere, numero);

        cliente_paga(&barbiere, numero);

        pausetta();
        pausetta();
    }
}

int main()
{
    // inizializzo thread e attr

    pthread_t clienti[N];

    int id[N];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_barbiere(&barbiere);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < N; i++)
    {
        id[i] = i;
        pthread_create(&clienti[i], &attr, cliente, &id[i]);
    }

    pthread_attr_destroy(&attr);

    sleep(5);

    printf("Fine Main\n");
    return 0;
}