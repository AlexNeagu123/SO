#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>

void handle_sigint(int nr_sem)
{
	printf("S-a terminat executia procesului.\n");
	exit(0);
}

void handle_sigusr1(int nr_sem)
{
	system("finger -p");
}

void handle_sigalrm(int nr_sem)
{
	int fd,cod_r;
	char ch;
	
	if(-1 == (fd = open("remember.txt",O_RDONLY)) )
	{
		perror("Eroare la deschiderea fisierului remembr.txt");  exit(1);
	}

	printf("Remember:\n");
	do{
		if(-1 == (cod_r = read(fd,&ch,sizeof(char))) )
		{
			perror("Eroare la citirea din fisier");  
			exit(2);
		}
		if(cod_r == 1) /* Dacă a citit cu succes un octet, îl afișăm. */
		{
			printf("%c",ch);
			fflush(stdout);
		}
	}while(cod_r !=0);  /* Executăm bucla de citire și afișare până întâlnim EOF în fișier. */
	printf("\n");

	close(fd);
}

int main()
{
	/* Mai întâi atașăm handler-ele pentru cele 3 tipuri de semnale pe care vrem să le tratăm în program. */
	signal(SIGINT, handle_sigint);
	signal(SIGUSR1, handle_sigusr1);
	signal(SIGALRM, handle_sigalrm);
    
	while(1)
	{
		alarm(10); /* Setăm alarma să "sune" (i.e. va genera semnalul SIGALRM) peste 10 secunde. */
		pause();   /* Suspendăm execuția până la primirea următorului semnal. */
	}

	return 0;
}
