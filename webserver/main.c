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
//selon ou on place le projet
#define DOCUMENT_ROOT "../ressources/"

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

char* lireMessageBienvenu () {
	int fd = open("../ressources/Bienvenue.html", O_RDONLY);
	int s = 0;
	char buf[BLOCK_SIZE];
	char *msg = malloc(1024);
	int size = 0;
	while((s=read(fd,&buf,BLOCK_SIZE))>0){
		msg = strcat(msg,buf);
		size = size + s;
	}
	msg = realloc(msg, size);
	return msg;
}

char *fgets_or_exit(char *buffer, int size, FILE *stream){
	buffer = fgets(buffer,size,stream);
	if(buffer == NULL){
		exit(0);
	}
	return buffer;
}

//fonction qui retourne la taille d’un fichier déjà ou- vert à partir de son descripteur
int get_file_size(int fd) {
	struct stat *buf = NULL;
	fstat(fd,buf);
    return buf->st_size;
}

void skip_headers(FILE *client){
	char buf[BLOCK_SIZE];
	while(strcmp(fgets_or_exit(buf,BLOCK_SIZE,client), "\r\n") != 0){}
}

void send_status(FILE *client, int code, const char *reason_phrase){
	fprintf(client,"HTTP/1.1 %d %s\r\n", code, reason_phrase);
}

void send_response(FILE *client, int code, const char *reason_phrase, const char *message_body){
	send_status(client,code,reason_phrase);
	fprintf(client, "Content-Lenght: %d\r\n\r\n", (int) strlen(message_body));
	fprintf(client, "%s\r\n", message_body);
}

char *rewrite_target(char *target){
	char *path = malloc(strlen(target)*sizeof(char));
	int i = 0;
	int idx = 1;
	while(target[idx] != '?' && i < (int)strlen(target)){
		path[i] = target[idx];
		i++;
		idx++;
	}
	return path;
}

FILE *check_and_open(const char *target, const char *document_root){

	struct stat *buf = NULL;
	char *fichier = malloc(sizeof(char)*(strlen(target)+strlen(document_root))+1);
	printf("fich: %s\n", fichier);	
	strcpy(fichier,document_root);
	printf("fich: %s\n", fichier);
	strcat(fichier,target);
	printf("fich: %s\n", fichier);

	//verifie si le fichier est regulier
	if (stat(fichier, buf) == -1)
    {
      perror(fichier);
      return NULL;
    }
 
    if (S_ISREG(buf->st_mode)){
    	FILE *fd = fopen(fichier,"r");
    	return fd;
    } else {
    	return NULL;
    }

}

int sendfile(int fdest, int fsource, int fileSize){
	char buffer[fileSize];
	int s;
	int total = 0;

	// On lit un premier morceau read retourne le nombre d'octet lu. Le
	// troisième paramètre donne la taille MAXIMALE à lire. Il se peut
	// donc que read lise MOINS que BLOCK_SIZE
	s = read(fsource, buffer, fileSize);
	// Tant qu'il y a des octets lus (0 -> fin de fichier, -1 -> erreur)
	while (s > 0)
	{
	  // On écrit le nombre d'octets lus dans le fichier de destination
	  // Le buffer contient les données lues par read
	  if (write(fdest, buffer, s) == -1)
	  {
	     perror("write");
	     return -1;
	  }
	  total += s;
	  // On lit le morceau suivant
	  s = read(fsource, buffer, BLOCK_SIZE);
	}
	if (s == -1)
	{
	  perror("read");
	  return -1;
	}
	return total;
}

int copy(FILE *in, FILE *out){
	fprintf(out, "Content-Lenght: %d\r\n\r\n", get_file_size(fileno(in)));
	fflush(out);
	return sendfile(fileno(out),fileno(in),get_file_size(fileno(in)));
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
			//char * nom = "Serveur : ";
			FILE * file = fdopen(socket_client,"w+");
			http_request requete;
			int bad_request = parse_http_request(fgets_or_exit(buf,BLOCK_SIZE,file),&requete);
			FILE * ressource= check_and_open(rewrite_target(requete.target),DOCUMENT_ROOT);

			if(bad_request==0){
				send_response(file,400,"Bad Request","400:Bad request\r\n");
			} else if (requete.method == HTTP_UNSUPPORTED){
				send_response(file, 405, "Method Not Allowed", "405:Method Not Allowed\r\n");
			} else if (ressource != NULL) {
				skip_headers(file);
				send_response(file, 200, "OK", lireMessageBienvenu());
				copy(ressource,file);
			} else {
				send_response(file, 404, "Not Found", "404:Not Found\r\n");
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