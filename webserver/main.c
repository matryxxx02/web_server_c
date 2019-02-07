# include "socket.h"
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <signal.h>
# include <sys/wait.h>
# include <stdlib.h>
#define BLOCK_SIZE 1024

void traitement_signal() {
	waitpid(-1,NULL,0);
}

void initialiser_signaux(void){
	//processus zombies
	struct sigaction sa;
	sa.sa_handler = traitement_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD , &sa, NULL) == -1) {
		perror("sigaction(SIGCHLD)");
	}

	//initialise le signal lorsqu'il n'y a plus declient
	if (signal(SIGPIPE , SIG_IGN) == SIG_ERR) {
		perror("signal");
	}
}

void messageBienvenu (int socket_client) {
	const char * message_bienvenue = "Bonjour,\n bienvenue sur notre serveur en construction !!\n";
	write(socket_client , message_bienvenue , strlen(message_bienvenue));
	
	const char * message_bienvenue1 = "C'est un projet pour le S4 !!\n C'est le debut du projet \n";
	write(socket_client , message_bienvenue1 , strlen(message_bienvenue1));
	
	const char * message_bienvenue2 = "Il nous faut 10 Lignes!!\n mais malheureusement je ne sais pas quoi dire\n";
	write(socket_client , message_bienvenue2 , strlen(message_bienvenue2));
	
	const char * message_bienvenue3 = "De grosse modif sont attendu\n Revenez plus tard \n Je m'occupe de l'echo: \n";
	write(socket_client , message_bienvenue3 , strlen(message_bienvenue3));
}

int main ( int argc , char ** argv ) {

	/* Arnold Robbins in the LJ of February ’95 , describing RCS */
	if ( argc > 1 && strcmp ( argv [1] , " - advice " ) == 0) {
		printf("Don ’t Panic !\n");
		return 42;
	}
	
	int socket_server = creer_serveur(8080);
	if(socket_server == -1){
		return 1;
	}

	initialiser_signaux();

	while(1){
		//int s = 0;
		char buf[BLOCK_SIZE];
		int socket_client = accept_(socket_server);		

		int pid = fork();
		if(pid==0){
			//dans le fils

			char * nom = "Serveur : ";
			FILE * file = fdopen(socket_client,"w+");
			if(strcmp(fgets(buf,BLOCK_SIZE,file), "GET / HTTP/1.1\r\n") == 0){
				while(strcmp(fgets(buf,BLOCK_SIZE,file), "\r\n") != 0){
					fprintf(file,"%s%s",nom,buf);
				}
				messageBienvenu(socket_client);
			}else {
			char * reponse = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Lenght: 17\r\n\r\n400 Bad Request\r\n";
			fprintf(file,"%s%s",nom,reponse);
			}
			exit(0);

		}else{
			//dans le pere
			close(socket_client);
		}

	}
	printf("Need an advice ?\n");
	return 0;
}