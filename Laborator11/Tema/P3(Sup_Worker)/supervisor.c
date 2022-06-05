/*
  Programul: supervisor.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#define MAX_NO_WORKERS      10
#define MAX_LENGTH_FILENAME 50

int main() {
	
	int nr = 0, k, NW = 0, i, pid, sum = 0;
	printf("Dati numarul de procese workers: ");
	while(1 != scanf("%d", &NW) ) { fprintf(stderr, "Trebuie sa introduceti un numar intreg pozitiv! Incercati din nou...\n"); }
	if( (NW < 1) || (NW > MAX_NO_WORKERS) )
	{
		fprintf(stderr,"Date de intrare incorecte.\n");  
		exit(1);
	}
	
	char fifos[NW + 1][MAX_LENGTH_FILENAME];
	for(int i = 0; i < NW; ++i) {
		sprintf(fifos[i], "fifo%d", i);
	}
	strcpy(fifos[NW], "fifo_out");

	for (i = 0; i <= NW; ++i) {
		if(-1 == mkfifo(fifos[i], 0600) ) {
			perror("Eroare la mkfifo");
			exit(2);
		}
	}
	
	for (i = 0; i < NW; ++i) {
		if(-1 == (pid = fork()) )
		{
			fprintf(stderr,"[Supervisor] Eroare la apelul fork() pentru procesul worker al %d-lea.\n", i);
			perror("Cauza erorii: ");
			exit(3);
		}
		
		if (pid == 0)
		{
			execl("worker", "Programul worker", fifos[i], fifos[NW], NULL);
			fprintf(stderr,"[Supervisor] Eroare la apelul execl() pentru procesul worker al %d-lea.\n", i);
			perror("Cauza erorii: ");
			exit(4);
		} 
	}
	
	FILE *head[NW];
	for(int i = 0; i < NW; ++i) {
		if(NULL == (head[i] = fopen(fifos[i], "w"))) {
			perror("Eroare la deschiderea capatului de scriere in fifo");
			exit(3);
		}
	}
	
	printf("Introduceti secventa de numere (0 pentru terminare) :\n");
	k = 0;
	
	do {
		while(1 != scanf("%d", &nr) ) { 
			fprintf(stderr, "Trebuie sa introduceti un numar intreg nenul! Incercati din nou...\n"); 
		}
		if(nr != 0) {
			fprintf(head[k], "%d ",nr);
			fflush(head[k]);
		}
		k = (k + 1) % NW;
	} while(nr != 0);
	
	for(int i = 0; i < NW; ++i) {
		fclose(head[i]);
	}
	
	FILE *fout = fopen(fifos[NW], "r"); 
	if(fout == NULL) {
		perror("Eroare la deschiderea capatului de scriere in fifo");
		exit(3);
	}
	
	for (i = 1; i <= NW; ++i) {
		wait(NULL);
		if (1 != fscanf(fout, "%d", &nr)) {
			fprintf(stderr,"[Supervisor] Eroare la citirea celei de a %d-a sume partiale.\n", i);
			exit(6);
		}
		else
			sum += nr;
	}
	fclose(fout);
	printf("[Supervisor] Suma calculata a secventei introduse este: %d.\n", sum);
	for (i = 0; i <= NW; ++i) {
		remove(fifos[i]);
	}
	return 0;
}


//RETINE: La canalele fifo mereu deschiderea are loc blocant. Grija la ordinea in care deschid!
