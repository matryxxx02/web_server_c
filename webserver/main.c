# include "socket.h"
# include <stdio.h>
# include <string.h>
# include <unistd.h>

int main ( int argc , char ** argv ) {

	/* Arnold Robbins in the LJ of February ’95 , describing RCS */
	if ( argc > 1 && strcmp ( argv [1] , " - advice " ) == 0) {
		printf("Don ’t Panic !\n");
		return 42;
	}
	
	int socket_server = creer_serveur(8080);
	int socket_client;

	while(1){

		socket_client = accept_(socket_server);

		/* On peut maintenant dialoguer avec le client */
		const char * message_bienvenue = "Bonjour, bienvenue sur mon serveur\n";
		write(socket_client , message_bienvenue , strlen(message_bienvenue));
	}


	printf("Need an advice ?\n");
	return 0;
}