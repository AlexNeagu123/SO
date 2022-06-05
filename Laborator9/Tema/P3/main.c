#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

void dialog_tata()
{
	// TODO: de implementat bucla dialogului pentru tata

	// Scrieți două bucle imbricate, conform indicațiilor cuprinse în diagramă!
	int fd;
	if(-1 == (fd = open("flag.bin", O_RDWR))) {
		perror("Eroare la deschiderea fisierului bin");
		exit(1);
	}	
	FILE *F = fopen("replici-parinte.txt", "r");
	if(F == NULL) {
		fprintf(stderr, "Eroare la deschiderea fisierului replici-parinte : %d ", errno);
		exit(2);
	}
	
	char replica[4096];
	int cnt = 0;
	char flag = '0';
	
	while(fgets(replica, 4096, F) != NULL) {
		
		// AFISARE REPLICA
		++cnt;
		printf("[R%d - parinte] %s", cnt, replica);
		
		if(flag == '2') {
			continue;
		}
		
		// YOUR TURN
		if(-1 == lseek(fd, 0, SEEK_SET)) {
			perror("Eroare la repozitionare");
			exit(3);
		}
		
		char dep = '1';
		if(-1 == write(fd, &dep, 1)) {
			perror("[Fiu] Eroare la scrierea in fisierul binar");
			exit(2);
		}
		flag = '1'; 
		
		// WAIT FOR MY TURN
		do { 
			sleep(0); 
			if(-1 == lseek(fd, 0, SEEK_SET)) {
				perror("Eroare la repozitionare");
				exit(3);
			}
			if(-1 == read(fd, &flag, 1)) {
				perror("[Parinte] Eroare la citirea din fisierul binar");
				exit(2);
			}
		} 
		while(flag == '1');
		sleep(2);
	}
	// Am terminat replicile setez flagul 2
	if(-1 == lseek(fd, 0, SEEK_SET)) {
		perror("Eroare la repozitionare");
		exit(3);
	}
	char dep = '2';
	if(-1 == write(fd, &dep, 1)) {
		perror("[Parinte] Eroare la scrierea in fisierul binar");
		exit(2);
	}
	if(-1 == close(fd)) {
		perror("[Parinte] Eroare la inchiderea fisierului binar");
		exit(3);
	}
	if(fclose(F) != 0) {
		fprintf(stderr, "Eroare la inchiderea fisierului replici_parinte : %d \n", errno);
		exit(3);
	} 
}

void dialog_fiu()
{
	// TODO: de implementat bucla dialogului pentru fiu

	// Scrieți două bucle imbricate, conform indicațiilor cuprinse în diagramă!
	int fd;
	if(-1 == (fd = open("flag.bin", O_RDWR))) {
		perror("Eroare la deschiderea fisierului bin");
		exit(1);
	}	
	
	FILE *F = fopen("replici-fiu.txt", "r");
	if(F == NULL) {
		fprintf(stderr, "Eroare la deschiderea fisierului relici_fiu : %d \n", errno);
		exit(2);
	}
	char replica[4096];
	int cnt = 0;
	char flag = '1';
	
	while(fgets(replica, 4096, F) != NULL) {
		
		if(flag == '2') {
			// AFISARE REPLICA 
			++cnt;
			printf("[R%d - Fiu] %s", cnt, replica);
			continue;
		}
		
		// WAIT FOR MY TURN
		flag = '0'; 
		do { 
			sleep(0); 
			if(-1 == lseek(fd, 0, SEEK_SET)) {
				perror("Eroare la repozitionare");
				exit(3);
			}
			if(-1 == read(fd, &flag, 1)) {
				perror("[Fiu] Eroare la citirea din fisierul binar");
				exit(2);
			}
		} 
		while(flag == '0');
		
		// AFISARE REPLICA 
		++cnt;
		printf("[R%d - Fiu] %s", cnt, replica);
		
		// YOUR TURN
		if(-1 == lseek(fd, 0, SEEK_SET)) {
			perror("Eroare la repozitionare");
			exit(3);
		}
		char dep = '0';
		if(-1 == write(fd, &dep, 1)) {
			perror("[Fiu] Eroare la scrierea in fisierul binar");
			exit(2);
		}
		sleep(2);
	}
	// Am terminat replicile setez flagul 2
	
	if(-1 == lseek(fd, 0, SEEK_SET)) {
		perror("Eroare la repozitionare");
		exit(3);
	}
	char dep = '2';
	if(-1 == write(fd, &dep, 1)) {
		perror("[Fiu] Eroare la scrierea in fisierul binar");
		exit(2);
	}
	if(-1 == close(fd)) {
		perror("[Fiu]Eroare la inchiderea fisierului binar");
		exit(3);
	}
	if(fclose(F) != 0) {
		fprintf(stderr, "Eroare la inchiderea fisierului replici_fiu : %d \n", errno);
		exit(3);
	} 
}


int main()
{
	pid_t pid_fiu;
	int fd;
	// fd - file descriptorul fisierului flag

	// TODO: de inițializat cu 0 fișierul flag.bin
	if(-1 == (fd = open("flag.bin", O_RDWR | O_CREAT | O_TRUNC, 0700))) {
		perror("Eroare la deschiderea fisierului bin");
		exit(1);
	}	
	char in = '0';
	if(-1 == write(fd, &in, 1)) {
		perror("Eroare la initializarea fisierului bin");
		exit(2);
	}
	if(-1 == close(fd)) {
		perror("Eroare la inchiderea fisierului binar");
		exit(3);
	}
	
	/* Crearea unui proces fiu. */
	if(-1 == (pid_fiu = fork()) )
	{
		perror("Eroare la fork");  
		return 1;
	}

	/* Ramificarea execuției în cele două procese, tată și fiu. */
	if(pid_fiu == 0)
	{   /* Zona de cod executată doar de către fiu. */
		printf("\n[P1] Procesul fiu, cu PID-ul: %d.\n", getpid());
		dialog_fiu();
	}
	else
	{   /* Zona de cod executată doar de către părinte. */
		printf("\n[P0] Procesul tata, cu PID-ul: %d.\n", getpid());
		dialog_tata();
	}

	/* Zona de cod comună, executată de către ambele procese */
	printf("Sfarsitul executiei procesului %s.\n\n", pid_fiu == 0 ? "fiu" : "parinte" );
	return 0;
}
