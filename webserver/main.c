# include "socket.h"
# include <stdio.h>
# include "http_parse.h"
# include <string.h>
# include <signal.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <fcntl.h>

#define BLOCK_SIZE 1024

void traitement_signal() {
	int s;
	waitpid(-1,&s,WNOHANG);
	//WifSignaled renvoie vrai si le fils s'est terminé à cause d'un signal -> erreur de segmentation
	if(WIFSIGNALED(s)){
		fprintf(stderr, "Processus arrété par le signal : %d\n", WTERMSIG(s)); // Numero 11 = erreur de segementation
	}

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
	int fd = open("../ressources/Bienvenue.html", O_RDONLY);
	int s = 0;
	char buf[BLOCK_SIZE];
	char * reponseHttp = "HTTP/1.1 200 OK\r\nContent-Lenght: 549\r\n\r\n";

	write(socket_client,reponseHttp,strlen(reponseHttp));
	while((s=read(fd,&buf,BLOCK_SIZE))>0){
		write(socket_client , buf , strlen(buf));
	}
}

char *fgets_or_exit(char *buffer, int size, FILE *stream){
	
	buffer = fgets(buffer,size,stream);

	if(buffer == NULL){
		printf("client déco !!\n");
		exit(0);
	}

	return buffer;
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
			http_request requete;
			if(parse_http_request(fgets_or_exit(buf,BLOCK_SIZE,file),&requete)){
				if(strcmp(requete.target, "/inexistant") == 0){
					char * reponse = "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Lenght: 15\r\n\r\n404 Not Found\r\n";
					fprintf(file,"%s%s",nom,reponse);
				}else {
					while(strcmp(fgets_or_exit(buf,BLOCK_SIZE,file), "\r\n") != 0){
						//Lignes ignorées
						//fprintf(file,"%s%s",nom,buf);
					}
					messageBienvenu(socket_client);
				}
			} else {
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