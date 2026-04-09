#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

#define P 3
#define E 4
#define M 2
#define N 5

void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}

struct palestra_t
{
    pthread_mutex_t mutex;
    pthread_cond_t attrezzi[N];
    int prenotazione_persona[P], occupati[N];
} palestra;

void init_palestra(struct palestra_t *p)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;
    pthread_mutex_init(&p->mutex, &m_attr);
    for (size_t i = 0; i < N; i++)
    {
        pthread_cond_init(&p->attrezzi[i], &c_attr);
        p->occupati[i] = 0;
    }

    for (size_t i = 0; i < P; i++)
    {
        p->prenotazione_persona[i] = -1;
    }

    pthread_mutexattr_destroy(&m_attr);
    pthread_condattr_destroy(&c_attr);
}

void usaatrezzo(struct palestra_t *p, int numeropersona, int tipoatrezzo)
{
    pthread_mutex_lock(&p->mutex);

    if (p->prenotazione_persona[numeropersona] == tipoatrezzo)
    {
        printf("UTENTE %d aveva PRENOTATO attrezzo %d\n", numeropersona, tipoatrezzo);
        p->prenotazione_persona[numeropersona] == -1;
    }
    else
    {
        while (p->occupati[tipoatrezzo] >= M)
        {
            pthread_cond_wait(&p->attrezzi[tipoatrezzo], &p->mutex);
        }
        printf("UTENTE %d trova attrezzo %d LIBERO\n", numeropersona, tipoatrezzo);
        p->occupati[tipoatrezzo]++;
    }

    pthread_mutex_unlock(&p->mutex);
}

void prenota(struct palestra_t *p, int numeropersona, int prossimoatrezzo)
{
    pthread_mutex_lock(&p->mutex);

    if (p->occupati[prossimoatrezzo] < M)
    {
        printf("PRENOTAZIONE di: %d dell' attrezzo %d andata a BUON FINE\n", numeropersona, prossimoatrezzo);
        p->occupati[prossimoatrezzo]++;
        p->prenotazione_persona[numeropersona] = prossimoatrezzo;
    }
    else
        printf("PRENOTAZIONE di: %d FALLITA\n", numeropersona);

    pthread_mutex_unlock(&p->mutex);
}

void fineuso(struct palestra_t *p, int numeropersona, int attrezzocorrente)
{
    pthread_mutex_lock(&p->mutex);

    p->occupati[attrezzocorrente]--;
    pthread_cond_signal(&p->attrezzi[attrezzocorrente]);

    pthread_mutex_unlock(&p->mutex);
}

void *persona(void *args)
{
    int numero = *(int *)args;

    int attrezzoCorrente = rand() % N;
    int prossimoAttrezzo = rand() % N;

    for (size_t i = E; i > 0; i--)
    {
        printf("%d VUOLE USARE attrezzo %d e PRENOTARE attrezzo %d\n", numero, attrezzoCorrente, prossimoAttrezzo);
        usaatrezzo(&palestra, numero, attrezzoCorrente);
        printf("%d sta USANDO attrezzo %d\n", numero, attrezzoCorrente);
        pausetta();

        if (i != 1)
        {
            prenota(&palestra, numero, prossimoAttrezzo);
        }
        printf("%d LIBERO attrezzo %d\n", numero, attrezzoCorrente);
        fineuso(&palestra, numero, attrezzoCorrente);

        if (i != 0)
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