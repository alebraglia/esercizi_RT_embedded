
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

// //////////////// SEMAFORI:
sem_t semaforo;

sem_init(&semaforo, 0, val_iniziale); // 1 se devo proteggere sezioni critiche | 0 se devo sincronizzare
sem_wait(semaforo)
sem_post(&semaforo)
sem_getvalue(&semaforo, &valore)

//  //////////////// MUTEX:

pthread_mutex_t mutex;
pthread_mutexattr_t m_attr;

Pthread_mutex_init(&mutex, m_attr);
pthread_mutex_lock(&mutex);
pthread_mutex_unlock(&mutex);

// //////////////// COND VAR:

pthread_cond_t cond;
pthread_condattr_t c_attr;

pthread_cond_init(&cond, c_attr);
pthread_cond_wait(&cond, &mutex);
pthread_cond_signal(&cond);
pthread_cond_broadcast(&cond);

/*
NOTA: usare while(!condizione){ Pthread_cond_wait() }
*/


// //////////////// SEMAFORI PRIVATI:

// Primo approccio: Preventive post

void alloca_risorsa(struct risorsa_t *r) {
    sem_wait(&r->mutex);
    if (condizione){
        //alloco la risorsa
        sem_post(&r->semaforo[i]); // preventiva
    } else {
        //non posso allocare, mi metto in attesa
        r->bloccati++;
    }
    sem_post(&r->mutex);
    sem_wait(&r->semaforo[i]);
}

void libera_risorsa(struct risorsa_t *r) {
    int i;
    sem_wait(&r->mutex);
    if (r->bloccati > 0) {
        i = processo da svegliare; 
        //alloco la risorsa ad i
        r->bloccati--;
        sem_post(&r->semaforo[i]);
    }

    sem_post(&r->mutex);
}


// Secondo approccio: Token pass

void acquisisce_risorsa(struct risorsa_t *r) {
    sem_wait(&r->mutex);
    if (mi devo bloccare) {
        r->bloccati++;
        sem_post(&r->mutex);
        sem_wait(&r->semaforo[i]);
        r->bloccati--;
    }
    // alloco risorse al processo

    sem_post(&r->mutex) //rilascio del mutex che mi è stato passato
}

void acquisisce_risorsa(struct risorsa_t *r){
    int i;
    sem_wait(&r->mutex);

    //rilascio risorsa

    if (devo svegliare qualcuno)
    {
        i= proc da svegliare;
        sem_post(r->semaforo[i]);
    }
    else sem_post(&r->mutex); // caso non devo svegliare nessuno
    
}

// /////////////////CODA FIFO:
/*
-next[i]: Contiene l'indice della busta successiva a quella i-esima. Sia vuota (testa = head) sia delle vuote (testa = free)

allinizio sarà 0 -> 1 -> 2 -> -1 (fine)

-head e tail: Puntano rispettivamente alla prima e all'ultima busta piene (quelle nella mailbox).

All'inzio saranno = -1

-free: Punta alla testa di una lista di buste vuote disponibili.

All'inizio sarà 0

-NESSUNO (-1): Indica la fine della lista.

*/

//se la coda è vuota head = tail = -1

// INSERIMENTO ELEMENTO --------------------------------------

// 1. Prendo la prima busta libera disponibile
int i = free; 

// 2. AGGIORNAMENTO FONDAMENTALE: free deve puntare alla prossima busta vuota
free = next[i]; 

// 3. Preparo la busta i (sarà l'ultima della mailbox, quindi next è -1)
next[i] = NESSUNO;

// 4. Inserimento in coda (FIFO)
if (head == NESSUNO) {
    // Caso coda vuota: la busta i diventa sia testa che coda
    head = i;
} else {
    // Caso coda con elementi: la vecchia coda ora punta alla nuova busta
    next[tail] = i;
}
tail = i; // La nuova busta diventa ufficialmente la coda

//-----------------------------------------------

//ESTRAZIONE DI UN ELEMENTO ----------------------------------------

// 1. Controllo se c'è qualcosa (gestito dal semaforo nel codice reale)
if (head == NESSUNO) {
    // attendi...
}

// 2. Prendo l'indice dalla testa
int i = head;

// 3. Aggiorno la testa: la nuova testa è quella che segue la busta i
head = next[i];

// 4. Se la coda è diventata vuota, resetto anche tail (opzionale ma pulito)
if (head == NESSUNO) {
    tail = NESSUNO;
}

// 5. Rilascio della busta: la rimetto nel pool 'free'
next[i] = free; // La busta i ora punta alla vecchia testa di free
free = i;       // La busta i diventa la nuova testa di free


//------------------------------------------------------------------