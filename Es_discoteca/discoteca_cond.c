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
    pthread_mutex_t mutex;
    pthread_cond_t cassa, cliente, fuori, servito;
    int b_c, primo, porta, occupata, biglietto_pronto;
} discoteca;

void init_discoteca(struct discoteca_t *d)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_cond_init(&d->cliente, &c_attr);
    pthread_cond_init(&d->fuori, &c_attr);
    pthread_cond_init(&d->cassa, &c_attr);
    pthread_mutex_init(&d->mutex, &m_attr);
    d->b_c = 0;
    d->occupata = 0;
    d->porta = 1;
    d->primo = 1;
    d->biglietto_pronto = 0;
}

void cliente_coda_fuori(struct discoteca_t *d)
{
    pthread_mutex_lock(&d->mutex);

    if (d->primo)
    {
        d->primo = 0;
        d->porta = 0;
        printf("PORTA CHIUSA\n");
        pthread_mutex_unlock(&d->mutex);
        return;
    }

    while (!d->porta)
    {

        printf("Cliente ASPETTA FUORI\n");
        pthread_cond_wait(&d->fuori, &d->mutex);
    }

    pthread_mutex_unlock(&d->mutex);
}

void cliente_esco_coda(struct discoteca_t *d)
{
    pthread_mutex_lock(&d->mutex);

    while (!d->biglietto_pronto)
    {
        pthread_cond_wait(&d->servito, &d->mutex);
    }
    printf("Cliente PRESO BIGLIETTO\n");

    d->biglietto_pronto = 0;
    d->b_c--;
    d->occupata = 0;

    if (d->b_c == 0)
    {
        d->porta = 1;
        printf("FINITI clienti dentro: APRO LA PORTA\n");
        pthread_cond_broadcast(&d->fuori);
    }

    pthread_mutex_unlock(&d->mutex);
}

void cliente_coda_dentro(struct discoteca_t *d)
{
    pthread_mutex_lock(&d->mutex);

    d->b_c++;
    pthread_cond_signal(&d->cliente);
    while (d->occupata)
    {
        pthread_cond_wait(&d->cassa, &d->mutex);
    }

    d->porta = 0;
    d->occupata = 1;
    pthread_mutex_unlock(&d->mutex);
}

void cassiera_attende_cliente(struct discoteca_t *d)
{
    pthread_mutex_lock(&d->mutex);

    pthread_cond_signal(&d->cassa);
    while (d->b_c == 0)
    {
        printf("CASSA ATTENDO cliente\n");
        pthread_cond_wait(&d->cliente, &d->mutex);
    }

    pthread_mutex_unlock(&d->mutex);
}

void cassiera_cliente_servito(struct discoteca_t *d)
{
    pthread_mutex_lock(&d->mutex);

    printf("CASSA EMETTO biglietto\n");
    d->biglietto_pronto = 1;
    pthread_cond_signal(&d->servito);

    pthread_mutex_unlock(&d->mutex);
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
        printf("CASSA SERVO cliente\n");
        cassiera_attende_cliente(&discoteca);
        pausetta();
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