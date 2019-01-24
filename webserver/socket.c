#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>

int creer_serveur(int port){
	//On récupére le descripteur du socket serveur
	int socket_serveur = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_serveur == -1){
		/* traitement de l’erreur */
		perror("problème lors de la création du socket_serveur");
	}

	struct sockaddr_in {
		sa_family_t			sin_family; /* address family : AF_INET */
		in_port_t				sin_port; 	/* port in network byte order */
		struct in_addr 	sin_addr; 	/* internet address */
	};

	/* Internet address . */
	struct in_addr {
		uint32_t 			s_addr;		/* address in network byte order */
	};

	//Attachement de la socket serveur sur toutes les interfaces
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET; /* Socket ipv4 */
	saddr.sin_port = htons(port); /* Port d’écoute */
	saddr.sin_addr.s_addr = INADDR_ANY; /* écoute sur toutes les interfaces */

	//rattachement de la socket a une adresse et un port
	if (bind(socket_serveur, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
		perror("erreur lors du bind du socker_serveur");
		/* traitement de l’erreur */
	}

	if ( listen ( socket_serveur , 10) == -1) {
		perror ( " erreur lors de l'execution de listen avec le socket_serveur " );
		/* traitement d ’ erreur */
	}

	int socket_client;
	socket_client = accept(socket_serveur, NULL, NULL);
	if (socket_client == -1){
		perror("erreur lors du accept");
		/* traitement d’erreur */
	}

	/* On peut maintenant dialoguer avec le client */
	const char *message_bienvenue = "Bonjour, bienvenue sur mon serveur\n";
	write(socket_client , message_bienvenue , strlen(message_bienvenue));

	return 0;
}
