/*
  Program: soluția incompletă a problemei [Exec command #3: ls ...; rm ... ].
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>  // necesar pentru PATH_MAX

int main(int argc, char* argv[])
{
	char nume_dir[PATH_MAX];
	int cod_term;

	// Pasul #1: obtinerea numelui directorului de prelucrat
	
	if(argc > 1)
	{
		strcpy(nume_dir,argv[1]);
	}
	else
	{
		printf("Introduceti numele directorului dorit:");  
		scanf("%s", nume_dir);
	}

	// Pasul #2: invocarea primei comenzi: ls -l nume_dir
	
	switch( fork() )
	{
		case -1:  perror("Eroare la primul fork");  exit(1);

		case  0:  execlp("ls","ls","-l", nume_dir, NULL);
		          perror("Eroare la primul exec");
		          exit(10);
		
		default:  wait(&cod_term);
		          if( WIFEXITED(cod_term) )
		              printf("Comanda ls a rulat, terminandu-se cu codul de terminare: %d.\n", WEXITSTATUS(cod_term) );
		          else
		              printf("Comanda ls a fost terminata fortat de catre semnalul: %d.\n", WTERMSIG(cod_term) );
	}
	
	char answer = '?';
	do {
		
		printf("Doriti stergerea directorului %s?\n", nume_dir);
		printf("Y = Da\n");
		printf("N = Nu\n");
		scanf("%c", &answer);
	
	} while(answer != 'Y' && answer != 'N');
	
	if(answer == 'N') {
		
		printf("Programul s-a incheiat. Utilizatorul nu doreste stergerea.\n");
		exit(0);
		
	}
	
	char *vec[5];
	
	switch( fork() ) {
		case -1: perror("Eroare la al doilea fork");
				 exit(1);
				 
	    case 0 : 
			     vec[0] = "rm";
			     vec[1] = "-r";
			     vec[2] = "-i";
			     vec[3] = nume_dir;
			     vec[4] = NULL;
				 execv("/usr/bin/rm", vec);
				 perror("Eroare la al doilea exec");
				 exit(10);
				 
		default: wait(&cod_term);
				 if(WIFEXITED(cod_term)) 
					printf("Comanda rm a rulat, terminandu-se cu codul de terminare: %d.\n", WEXITSTATUS(cod_term));
				else 
					printf("Comanda ls a fost terminata fortat de catre semnalul: %d.\n", WTERMSIG(cod_term) );
	}
	
}
