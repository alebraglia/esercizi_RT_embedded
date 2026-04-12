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
    pthread_mutex_t mutex;
    pthread_cond_t fuori, cassa, barbiere;
    int coda, cassa_libera, tagliando;
} barbiere;

void init_barbiere(struct barbiere_t *b)
{
    pthread_cond_init(&b->barbiere, NULL);
    pthread_cond_init(&b->cassa, NULL);
    pthread_cond_init(&b->fuori, NULL);
    pthread_mutex_init(&b->mutex, NULL);
    b->coda = 0;
    b->cassa_libera = 1;
    b->tagliando = 0;
}

void cliente_arriva(struct barbiere_t *b, int cliente)
{
    pthread_mutex_lock(&b->mutex);

    while (b->coda >= 4)
    {
        printf("Cliente %d ASPETTA FUORI\n", cliente);
        pthread_cond_wait(&b->fuori, &b->mutex);
    }
    b->coda++;

    pthread_mutex_unlock(&b->mutex);
}

void cliente_entra(struct barbiere_t *b, int cliente)
{
    pthread_mutex_lock(&b->mutex);
    printf("Cliente %d ENTRATO\n", cliente);

    while (b->tagliando >= 3)
    {
        printf("Cliente %d ASPETTA BARBIERE\n", cliente);
        pthread_cond_wait(&b->barbiere, &b->mutex);
    }
    
    b->coda--;
    b->tagliando++;

    pthread_cond_signal(&b->fuori);
    pthread_mutex_unlock(&b->mutex);

    printf("Cliente %d STA TAGLIANDO BARBA\n", cliente);
    for (size_t i = 0; i < SHAVING_ITERATIONS; i++)
    {
        pausetta();
    }

    pthread_mutex_lock(&b->mutex);

    printf("Cliente %d FINITO TAGLIO\n", cliente);
    b->tagliando--;

    pthread_cond_signal(&b->barbiere);
    pthread_mutex_unlock(&b->mutex);
}

void cliente_paga(struct barbiere_t *b, int cliente)
{
    pthread_mutex_lock(&b->mutex);

    while (!b->cassa_libera)
    {
        printf("Cliente %d ASPETTA CASSA\n", cliente);
        pthread_cond_wait(&b->cassa, &b->mutex);
    }
    b->cassa_libera = 0;

    pthread_mutex_unlock(&b->mutex);
    printf("Cliente %d PAGA\n", cliente);
    for (size_t i = 0; i < PAYING_ITERATIONS; i++)
    {
        pausetta();
    }
    pthread_mutex_lock(&b->mutex);

    printf("Cliente %d ESCE\n", cliente);
    b->cassa_libera = 1;
    pthread_cond_signal(&b->cassa);
    pthread_mutex_unlock(&b->mutex);

}

void *cliente(void *arg)
{
    int numero = *(int *)arg;
    while (1)
    {
        pausetta();
        printf("Cliente %d ARRIVA\n", numero);
        cliente_arriva(&barbiere, numero);

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