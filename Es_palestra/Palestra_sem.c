#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define M 2
#define N 6
#define E 4
#define P 3

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct Palestra
{
    sem_t sem_attrezzi[N], mutex;
    int attrezzi[N];
    int coda_attrezzi[N];
    int prenotazioni[P];
} palestra;

void init_palestra(struct Palestra *p)
{
    for (int i = 0; i < N; i++)
    {
        sem_init(&p->sem_attrezzi[i], 0, 0);
        p->attrezzi[i] = M;
        p->coda_attrezzi[i] = 0;
    }
    for (int i = 0; i < P; i++)
    {
        p->prenotazioni[i] = -1;
    }
    sem_init(&p->mutex, 0, 1);
}

void prenota(struct Palestra *p, int persona, int m)
{
    sem_wait(&p->mutex);

    if (&p->attrezzi[m] > 0)
    {
        p->prenotazioni[persona] = m;
        p->attrezzi[m]--;
    }
    sem_post(&p->mutex);
}

void usaatrezzo(struct Palestra *p, int persona, int m)
{
    sem_wait(&p->mutex);
    if (&p->prenotazioni[persona] == m || &p->attrezzi[m] > 0)
    {
        if (&p->prenotazioni[persona] != m) p->attrezzi[m]--;

        if (&p->prenotazioni[persona] == m) p->prenotazioni[persona] = -1;

        sem_post(&p->sem_attrezzi[m]); // post preventiva

    } else {
        printf("%d si blocca in coda per attrezzo %d\n", persona, m);
        p->coda_attrezzi[m]++;
    }
    sem_post(&p->mutex);
    sem_wait(&p->sem_attrezzi[m]);
}

void fineuso(struct Palestra *p, int persona, int m){
    sem_wait(&p->mutex);

    printf("%d ha finito di usare attrezzo %d\n", persona, m);

    if (p->coda_attrezzi[m] > 0)
    {
        p->coda_attrezzi[m]--;
        sem_post(&p->sem_attrezzi[m]);
    }
    else p->attrezzi[m]++;
    
    sem_post(&p->mutex);

}

void *persona(void *args)
{
    int numero = *(int*)args;

    int attrezzoCorrente = rand() % N;
    int prossimoAttrezzo = rand() % N;

    for (size_t i = E; i > 0; i--)
    {
        printf("%d usa attrezzo %d e prenota attrezzo %d\n", numero, attrezzoCorrente, prossimoAttrezzo);
        usaatrezzo(&palestra, numero, attrezzoCorrente);
        printf("%d sta usando attrezzo %d\n", numero, attrezzoCorrente);
        pausetta();

        if (i!=1)
        {
            prenota(&palestra, numero, prossimoAttrezzo);
        }
        fineuso(&palestra, numero, attrezzoCorrente);

        if (i!=0)
        {
            attrezzoCorrente = prossimoAttrezzo;
            prossimoAttrezzo = rand() % N;
        }
        pausetta();
        
    }

    pthread_exit(NULL);
}

int main()
{
    // inizializzo thread e attr
    pthread_t persone[P];
    int id_persone[P];

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    srand(555);
    init_palestra(&palestra);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    printf("creo i thread\n");
    for (int i = 0; i < P; i++)
    {
        id_persone[i] = i;
        pthread_create(&persone[i], &attr, persona, &id_persone[i]);
    }
    
    pthread_attr_destroy(&attr);

    sleep(5);

    printf("Fine Main\n");
    return 0;
}