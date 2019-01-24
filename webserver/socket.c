#include <stdio.h>
#include <sys/socket.h>

int creer_serveur(int port){
	//s descripteur du socket serveur
	int socket_serveur = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_serveur == -1){
		/* traitement de l’erreur */
		perror("socket_serveur");
	}

	//Attachement de la socket serveur sur toutes les interfaces
		struct sockaddr_in saddr{
		saddr.sin_family = AF_INET; /* Socket ipv4 */
		saddr.sin_port = htons(8080); /* Port d’écoute */
		saddr.sin_addr.s_addr = INADDR_ANY; /* écoute sur toutes les interfaces */
	}


	//rattachement de la socket a une adresse et un port
	if (bind(socket_serveur, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
		perror("bind socker_serveur");
		/* traitement de l’erreur */
	}
}
