//FIFO BARBERSHOP PROBLEM
//SECAO 5.3

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_CLIENTS 100                                         //Quantidade de threads cliente a serem criadas
#define MAX_SEATS 20                                            //Quantidade de cadeiras disponiveis na barbearia

pthread_mutex_t seatsMutex = PTHREAD_MUTEX_INITIALIZER;         //Mutex para as cadeiras da barbearia
pthread_mutex_t customersMutex = PTHREAD_MUTEX_INITIALIZER;     //Mutex para os clientes da barbearia
pthread_mutex_t barberMutex = PTHREAD_MUTEX_INITIALIZER;        //Mutex para o barbeiro
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;         //Mutex para o barbeiro

pthread_cond_t customerCond = PTHREAD_COND_INITIALIZER;         //Cond para cliente chegando na barbearia
pthread_cond_t barberCond = PTHREAD_COND_INITIALIZER;           //Cond para o barbeiro ficando livre

int customers = 0;                                              //Quantidade de clientes na barbearia no momento que ela abre (zero)
int barberCutting = 1;                                          //Disponibilidade do barbeiro (1 = disponivel 0 = indisponivel)
int left = 0;                                                   //Quantidade de clientes que foram embora pois a quantidade maxima ja havia sido alcançada
int cuts = 0;                                                   //Quantidade de clientes que conseguiram cortar o cabelo

long queue[MAX_CLIENTS];                                        //Fila de clientes
int rear = -1;                                                  //Final da fila
int front = -1;                                                 //Inicio da fila

void enqueue(long tid)                                          //Adiciona pessoa no final da fila
{
    if(rear == MAX_CLIENTS-1)                                   //Caso a fila esteja cheia, erro
    {
        printf("Queue ERROR: Nao foi possivel adicionar %ld na fila.\n",tid);
    }
    else
    {
        if(front == -1)
        {
            front=0;
        }
        rear++;
        queue[rear] = tid;
    }
}

void dequeue()                                                  //Move a posicao da frente da fila
{
    if (front == -1)                                            //Caso a fila esteja vazia, erro
    {
        printf("Queue ERROR: Nao foi possivel utilizar o metodo dequeue\n");
    }
    else
    {
        front++;
        if (front > rear)
        {
            front = rear = -1;
        }
    }
}

void showQueue()                                                //Imprime a fila
{
    printf("-----------------\n");
    printf("Fila:\n");

    for(int i = front; i <= rear; i++)
    {
        printf("%i : %ld\n",i-front+1,queue[i]);
    }
    printf("-----------------\n");
}

void cutHair()                                                  //Funcao de cortar o cabelo
{
    int worktime = (rand() % 1000 + 100);                       //Tempo do corte
    printf("Barbeiro: cortando cabelo por %d ms...\n",worktime);
    usleep(worktime);
    printf("Barbeiro: corte concluido!\n");
}

void *customer(void *arg)
{
    long tid = pthread_self();
    printf("Cliente %ld: indo para a barbearia...\n",tid);
    while(1)
    {
        pthread_mutex_lock(&customersMutex);
        if(MAX_SEATS <= customers)                              //Caso a quantidade de clientes seja excedida
        {
            printf("Cliente %ld: indo embora pois nao tem vaga.\n",tid);
            left++;
            pthread_mutex_unlock(&customersMutex);
            pthread_exit(NULL);
        }
        else
        {                                                       //Caso consiga entrar na barbearia
            customers++;
            printf("Cliente %ld: entrou na barbearia.\n",tid);
            pthread_cond_signal(&customerCond);
            pthread_mutex_unlock(&customersMutex);

            pthread_mutex_lock(&queueMutex);                    //Adiciona cliente na fila
                enqueue(tid);
            pthread_mutex_unlock(&queueMutex);

            pthread_mutex_lock(&barberMutex);
                if(barberCutting == 0)                          //Caso o barbeiro esteja ocupado, aguarde
                {
                    printf("Cliente %ld: barbeiro ocupado, vou esperar terminar.\n",tid);
                    pthread_cond_wait(&barberCond,&barberMutex);
                    printf("Cliente %ld: barbeiro nao esta mais ocupado!\n",tid);
                }
            pthread_mutex_unlock(&barberMutex);

            while(queue[front] != tid)                          //Enquanto nao e o proximo da fila, aguarde
            {
                usleep(100);
            }

            showQueue();                                        //Imprime a fila

            pthread_mutex_lock(&barberMutex);
                barberCutting = 1;
            pthread_mutex_unlock(&barberMutex);

            pthread_mutex_lock(&queueMutex);
                dequeue();                                      //Tira cliente da fila
            pthread_mutex_unlock(&queueMutex);

            pthread_exit(NULL);
        }
    }
}

void *barber(void *arg)
{
    while(cuts+left<MAX_CLIENTS)
    {
        pthread_mutex_lock(&seatsMutex);
        while(customers==0)                                     //Enquanto nao ha nenhum cliente, aguarde
        {
            printf("Barbeiro: dormindo...\n");
            pthread_cond_wait(&customerCond,&seatsMutex);
        }
        pthread_mutex_unlock(&seatsMutex);

        pthread_mutex_lock(&customersMutex);
            customers--;                                        //Comecou a cortar o cabelo de um cliente
            pthread_mutex_lock(&barberMutex);
                barberCutting = 0;                              //Disponibilidade do barbeiro (1 = disponivel 0 = indisponivel)
            pthread_mutex_unlock(&barberMutex);
        pthread_cond_signal(&barberCond);                       //Sinaliza que o barbeiro esta ocupado
        pthread_mutex_unlock(&customersMutex);

        cutHair();
        cuts++;
        printf("Barbeiro: ja cortei %d cabelos e ja sairam %d clientes. Tem %d clientes aguardando.\n",cuts,left,customers);
    }
    printf("Barbeiro: Barbearia fechada.\n");
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t barberThread;
    pthread_t customerThreads[MAX_CLIENTS];

    int createBarberThreadTime = (rand() % 100);
    pthread_create(&barberThread, NULL, barber, NULL);
    usleep(createBarberThreadTime);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        int createCustomerThreadTime = (rand() % 100);
        pthread_create(&customerThreads[i], NULL, customer, NULL);
        usleep(createCustomerThreadTime);
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
        pthread_join(customerThreads[i],NULL);

    pthread_join(barberThread,NULL);
}
