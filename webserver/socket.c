#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int creer_serveur(int port){
	//On récupére le descripteur du socket serveur
	int socket_serveur = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_serveur == -1){
		/* traitement de l’erreur */
		perror("problème lors de la création du socket_serveur");
		return -1;
	}

	//Attachement de la socket serveur sur toutes les interfaces
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET; /* Socket ipv4 */
	saddr.sin_port = htons(port); /* Port d’écoute */
	saddr.sin_addr.s_addr = INADDR_ANY; /* écoute sur toutes les interfaces */

	//rattachement de la socket a une adresse et un port
	if (bind(socket_serveur, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
		perror("erreur lors du bind du socker_serveur");
		return -1;
		/* traitement de l’erreur */
	}

	if (listen(socket_serveur, 10) == -1) {
		perror ( " erreur lors de l'execution de listen avec le socket_serveur " );
		return -1;
	}

	return socket_serveur;
}

int accept_(int socket_serveur){

	int socket_client;
	socket_client = accept(socket_serveur, NULL, NULL);
	if (socket_client == -1){
		perror("erreur lors du accept");
		return -1;
		/* traitement d’erreur */
	}

	return socket_client;

}

