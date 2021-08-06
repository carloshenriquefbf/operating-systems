#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

int comandoParaExecutar = 0;

void term_handler (int sigNumber){
    printf("Finalizando o disparador...\n");
    exit(0);
}

void usr1_handler (int sigNumber){
    printf("\n--------TAREFA 1---------\n");

    int descritoresPipe[2];

    printf("\nProcesso pai (pid: %d) criando o pipe...\n", getpid());

    pipe(descritoresPipe);

    printf("\nPipe criado!\n");

    printf("\ndescritoresPipe[0]: %d\n", descritoresPipe[0]);
    printf("\ndescritoresPipe[1]: %d\n", descritoresPipe[1]);

    printf("\nExecutando fork do pai (pid: %d) \n", getpid());

    int pidFilho = fork();

    if (pidFilho == 0) {
        printf("\n---------FILHO-----------\n");
        printf("\nSou o filho, pid: %d\n", getpid());

        srand(time(NULL));

        int random = rand() % 100 +1;

        printf("\nNumero sorteado: %d\n",random);

        printf("\nEnviando numero sorteado para o pai...\n");

        write(descritoresPipe[1], &random, sizeof(int));

        printf("\nEnviado!\n");

        printf("\nFechando as portas do pipe abertas...\n");

        close(descritoresPipe[1]);

        printf("\nPorta de escrita fechada!\n");

        close(descritoresPipe[0]);

        printf("\nPorta de leitura fechada!\n");
        exit(0);

    } else {
        waitpid(pidFilho,NULL,0);
        printf("\n-----------PAI-----------\n");
        printf("\nSou o pai, pid: %d\n", getpid());

        printf("\nLendo o numero sorteado enviado via pipe pelo filho...\n");

        read(descritoresPipe[0],&comandoParaExecutar,sizeof(int));

        printf("\nLeitura completa!\n");

        printf("\nNumero escrito pelo filho: %d\n", comandoParaExecutar);

        printf("\nFechando as portas do pipe abertas...\n");

        close(descritoresPipe[1]);

        printf("\nPorta de escrita fechada!\n");

        close(descritoresPipe[0]);

        printf("\nPorta de leitura fechada!\n");
    }
    printf("\n-----FIM DA TAREFA 1-----\n");
}

void usr2_handler (int sigNumber){
    printf("\n--------TAREFA 2---------\n");

    int pidFilho = fork();

    if (pidFilho == 0) {
        printf("\n---------FILHO-----------\n");
        printf("\nSou o filho, pid: %d\n", getpid());

        if(comandoParaExecutar == 0)
            printf("\nNao ha comando para executar");

        if(comandoParaExecutar != 0 && comandoParaExecutar % 2 == 0) {
            execl ( "/bin/ping", "ping", "-c 5", "8.8.8.8",NULL);
        }

        if(comandoParaExecutar != 0 && comandoParaExecutar % 2 != 0) {
            execl ( "/bin/ping", "ping", "-c 5", "-i 2", "paris.testdebit.info",NULL);
        }

        exit(0);
    } else {
        waitpid(pidFilho,NULL,0);
        printf("\n-----------PAI-----------\n");
        printf("\nSou o pai, pid: %d\n", getpid());
    }
    printf("\n-----FIM DA TAREFA 2-----\n");
}

void esperandoPorSinal (){
    printf("\nEsperando por sinal...\n");
    pause();
}

int main(void) {
    signal(SIGTERM, term_handler);
    signal(SIGUSR1, usr1_handler);
    signal(SIGUSR2, usr2_handler);

    pid_t pid = getpid();
    printf ("\nPid do processo: %d\n", pid);

    while(1)
        esperandoPorSinal();
}