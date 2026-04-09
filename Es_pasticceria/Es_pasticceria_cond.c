#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define N 10

struct pasticceria_t
{
    pthread_mutex_t mutex;
    pthread_cond_t cliente, commesso, full, empty;
    int torte, clienti, commesso_pronto;

} pasticceria;

void init_pasticceria(struct pasticceria_t *p)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;
    pthread_mutex_init(&p->mutex, &m_attr);
    pthread_cond_init(&p->cliente, &c_attr);
    pthread_cond_init(&p->commesso, &c_attr);
    pthread_cond_init(&p->full, &c_attr);
    pthread_cond_init(&p->empty, &c_attr);

    pthread_mutexattr_destroy(&m_attr);
    pthread_condattr_destroy(&c_attr);
}

void cuoco_inizio_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);
    while (p->torte > N)
    {
        pthread_cond_wait(&p->empty, &p->mutex);
    }
    pthread_mutex_unlock(&p->mutex);
}

void cuoco_fine_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);
    p->torte++;
    pthread_cond_signal(&p->full);
    pthread_mutex_unlock(&p->mutex);
}

void commesso_prende_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);
    while (p->torte == 0)
    {
        printf("COMMESSO: TORTE FINITE\n");
        pthread_cond_wait(&p->full, &p->mutex);
    }
    p->torte--;
    pthread_cond_signal(&p->empty);
    pthread_mutex_unlock(&p->mutex);
}

void commesso_vendo_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);
    
    p->commesso_pronto = 1;
    while (!p->clienti)
    {
        pthread_cond_wait(&p->cliente, &p->mutex);
    }
    
    pthread_cond_signal(&p->commesso);

    pthread_mutex_unlock(&p->mutex);
}

void cliente_acquisto(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);
    
    p->clienti++;
    pthread_cond_signal(&p->cliente);
    while (!p->commesso_pronto)
    {
        pthread_cond_wait(&p->commesso, &p->mutex);        
    }
    
    p->commesso_pronto = 0;
    p->clienti--;
    pthread_mutex_unlock(&p->mutex);
}

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

void *cuoco(void *args)
{
    while (1)
    {
        pausetta();
        cuoco_inizio_torta(&pasticceria);
        cuoco_fine_torta(&pasticceria);
        printf("cuoco ha preparato una torta\n");
    }
}

void *commesso(void *args)
{
    while (1)
    {
        commesso_prende_torta(&pasticceria);
        printf("COMMESSO: vuole vendere una torta\n");
        commesso_vendo_torta(&pasticceria);
        pausetta();
    }
}

void *cliente(void *args)
{
    while (1)
    {
        cliente_acquisto(&pasticceria);
        printf("CLIENTE ACQUISTATO TORTA\n");
        pausetta();
    }
}

int main()
{
    // inizializzo thread e attr
    pthread_t Tcuoco;
    pthread_t Tcommesso;
    pthread_t Tcliente1;
    pthread_t Tcliente2;
    pthread_t Tcliente3;

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_pasticceria(&pasticceria);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    pthread_create(&Tcuoco, &attr, cuoco, NULL);
    pthread_create(&Tcommesso, &attr, commesso, NULL);
    pthread_create(&Tcliente1, &attr, cliente, NULL);
    pthread_create(&Tcliente2, &attr, cliente, NULL);
    pthread_create(&Tcliente3, &attr, cliente, NULL);
    pthread_attr_destroy(&attr);

    sleep(2);

    printf("Fine Main\n");
    return 0;
}