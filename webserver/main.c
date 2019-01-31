# include "socket.h"
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <signal.h>

#define BLOCK_SIZE 1024

void initialiser_signaux(void){
	if (signal(SIGPIPE , SIG_IGN) == SIG_ERR) {
		perror("signal");
	}
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

	int idx = 0;

	while(idx<5){
		idx++;
		int socket_client = accept_(socket_server);		
		/* On peut maintenant dialoguer avec le client */
		const char * message_bienvenue = "Bonjour,\n bienvenue sur notre serveur en construction !!\n";
		write(socket_client , message_bienvenue , strlen(message_bienvenue));
		sleep(1);
		const char * message_bienvenue1 = "C'est un projet pour le S4 !!\n C'est le debut du projet \n";
		write(socket_client , message_bienvenue1 , strlen(message_bienvenue1));
		sleep(1);
		const char * message_bienvenue2 = "Il nous faut 10 Lignes!!\n mais malheureusement je ne sais pas quoi dire\n";
		write(socket_client , message_bienvenue2 , strlen(message_bienvenue2));
		sleep(1);
		const char * message_bienvenue3 = "De grosse modif sont attendu\n Revenez plus tard \n Je m'occupe de l'echo: \n";
		write(socket_client , message_bienvenue3 , strlen(message_bienvenue3));

		//pour la fonction echo plus tard :
		int s;
		char buf[BLOCK_SIZE];
		while((s=read(socket_client,&buf,BLOCK_SIZE))>0){
			write(socket_client,&buf,s);
		}
		
	}


	printf("Need an advice ?\n");
	return 0;
}