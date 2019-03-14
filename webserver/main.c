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

char *fgets_or_exit(char *buffer, int size, FILE *stream){
	buffer = fgets(buffer,size,stream);
	if(buffer == NULL){
		exit(0);
	}
	return buffer;
}

//fonction qui retourne la taille d’un fichier déjà ou- vert à partir de son descripteur
int get_file_size(int fd) {
	struct stat buf;
	fstat(fd,&buf);
    return buf.st_size;
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
	fprintf(client, "Content-Lenght: %d\r\n\r\n", (int)strlen(message_body));
	fprintf(client, "%s\r\n", message_body);
}

char *rewrite_target(char *target){
	int idx = 0;
	while(target[idx] != '?' && idx < (int)strlen(target)){
		if(target[idx] == '.' && idx+1 < (int)strlen(target) && target[idx+1] == '.'){
			target[idx] = '/';
			target[idx+1] = '/';
		}
		idx++;
	}
	if(target[idx] == '?')
		target[idx] = '\0';
	if(target[idx-1] == '/'){
		strcat(target, "index.html");
	}
	return target;
}

	FILE *check_and_open(const char *target, const char *document_root){

		struct stat buf;
		char path [sizeof(char)*(strlen(target)+strlen(document_root))+1];
		snprintf(path, sizeof(path), "%s%s", document_root, target);
		//verifie si le path est regulier
		if (stat(path, &buf)==-1)
	    {
	      perror(path);
	      return NULL;
	    }
	    if (S_ISREG(buf.st_mode)){
	    	FILE *fd = fopen(path,"r");
	    	return fd;
	    } else {
	    	return NULL;
	    }
	}

int sendfile(int fdest, int fsource, int fileSize){
	char buffer[fileSize];
	int s;
	int total = 0;

	s = read(fsource, buffer, fileSize);
	while (s > 0)
	{
	  if (write(fdest, buffer, s) == -1)
	  {
	     perror("write");
	     return -1;
	  }
	  total += s;
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
	send_status(out,200,"OK");
	fprintf(out, "Content-Lenght: %d\r\n\r\n", get_file_size(fileno(in)));
	fflush(out);
	return sendfile(fileno(out),fileno(in),get_file_size(fileno(in)));
}

int main (int argc , char ** argv) {

	/* Arnold Robbins in the LJ of February ’95 , describing RCS */
	if ( argc > 1 && strcmp ( argv [1] , " - advice " ) == 0) {
		printf("Don ’t Panic !\n");
		return 42;
	}
	
	char * DOCUMENT_ROOT = "../ressources/";
	if(argc > 1){
		printf("%s\n", argv[1]);
		DOCUMENT_ROOT = strcat(argv[1],"/");
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