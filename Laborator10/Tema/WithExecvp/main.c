#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>

// VARIANTA CU EXECVP

int main() {
	
	char Path[PATH_MAX] = "/etc/group";
	char *vec[6];
	int codTerm;
	
	vec[0] = "cut";
	vec[1] = "-f1,3";
	vec[2] = "-d:";
	vec[3] = "--output-delimiter= - ";
	vec[4] = Path;
	vec[5] = NULL;
	
	switch(fork()) {
		
		case -1: perror("Eroare la fork"); 
		         exit(2);
		         
		case 0:  execvp("cut", vec);
				 perror("Eroare la apelul execv");
				 exit(10);
	    
	    default:  wait(&codTerm);
				  if(WIFEXITED(codTerm)) {
					printf("Comanda cut a rulat, terminandu-se cu codul de terminare: %d.\n", WEXITSTATUS(codTerm));
				  } else {
					printf("Comanda ls a fost terminata fortat de catre semnalul: %d.\n", WTERMSIG(codTerm));
				  }
	}
}
