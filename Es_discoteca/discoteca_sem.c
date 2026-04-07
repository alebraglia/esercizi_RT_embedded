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

struct discoteca_t
{
    sem_t cassa, cliente, fuori, servito, mutex;
    int b_c, primo, porta, b_f;
} discoteca;

void init_discoteca(struct discoteca_t *d)
{

    sem_init(&d->cliente, 0, 0);
    sem_init(&d->fuori, 0, 0);
    sem_init(&d->cassa, 0, 0);
    sem_init(&d->servito, 0, 0);
    sem_init(&d->mutex, 0, 1);
    d->b_c = 0;
    d->porta = 1;
    d->primo = 1;
    d->b_f = 0;
}

void cliente_coda_fuori(struct discoteca_t *d)
{
    sem_wait(&d->mutex);

    if (d->primo)
    {
        d->primo = 0;
        d->porta = 0;
        printf("PORTA CHIUSA\n");
        sem_post(&d->mutex);
        return;
    }

    if (!d->porta)
    {
        printf("Cliente ASPETTA FUORI\n");
        d->b_f++;
        sem_post(&d->mutex);
        sem_wait(&d->fuori);
        sem_wait(&d->mutex);   
    }

    sem_post(&d->mutex);
}

void cliente_esco_coda(struct discoteca_t *d)
{
    sem_wait(&d->mutex);

    sem_post(&d->mutex);
    sem_wait(&d->servito);
    sem_wait(&d->mutex);
    printf("Cliente PRESO BIGLIETTO\n");

    d->b_c--;

    if (d->b_c == 0)
    {
        d->porta = 1;
        printf("FINITI clienti dentro: APRO LA PORTA\n");
        while (d->b_f > 0)
        {
            d->b_f--;
            sem_post(&d->fuori);
        }
    }

    sem_post(&d->mutex);
}

void cliente_coda_dentro(struct discoteca_t *d)
{
    sem_wait(&d->mutex);

    d->b_c++;

    sem_post(&d->cliente);

    sem_post(&d->mutex);
    sem_wait(&d->cassa);
    sem_wait(&d->mutex);

    sem_post(&d->mutex);
}

void cassiera_attende_cliente(struct discoteca_t *d)
{
    sem_wait(&d->mutex);
    sem_post(&d->cassa);
    printf("CASSA ATTENDO cliente\n");

    if (d->porta)
    {
        printf("CASSA CHIUDO porta\n");
        d->porta = 0;
    }
    
    sem_post(&d->mutex);
    sem_wait(&d->cliente);
}

void cassiera_cliente_servito(struct discoteca_t *d)
{
    printf("CASSA EMETTO biglietto\n");
    sem_post(&d->servito);
}

void *cliente(void *arg)
{
    int numero = *(int *)arg;

    pausetta();
    printf("CLIENTE %d ARRIVA all'ingresso\n", numero);
    cliente_coda_fuori(&discoteca);
    printf("CLIENTE %d ENTRA a fare biglietto\n", numero);
    cliente_coda_dentro(&discoteca);
    printf("CLIENTE %d PAGA\n", numero);
    cliente_esco_coda(&discoteca);
    printf("CLIENTE %d VA A BALLARE\n", numero);
}

void *cassiera(void *arg)
{
    while (1)
    {
        cassiera_attende_cliente(&discoteca);
        cassiera_cliente_servito(&discoteca);
        printf("CASSA aggiusto contanti\n");
        pausetta();
    }
}

int main()
{
    // inizializzo thread e attr
    pthread_t clienti[N];
    pthread_t cassa;
    int id_cliente[N];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_discoteca(&discoteca);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < N; i++)
    {
        id_cliente[i] = i;
        pthread_create(&clienti[i], &attr, cliente, &id_cliente[i]);
    }

    pthread_create(&cassa, &attr, cassiera, NULL);
    pthread_attr_destroy(&attr);

    sleep(2);

    printf("Fine Main\n");
    return 0;
}