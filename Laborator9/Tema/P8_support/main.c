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

char *flag_addr;
void dialog_tata()
{
	// TODO: de implementat bucla dialogului pentru tata

	// Scrieți două bucle imbricate, conform indicațiilor cuprinse în diagramă!
	
	FILE *F = fopen("replici-parinte.txt", "r");
	if(F == NULL) {
		fprintf(stderr, "Eroare la deschiderea fisierului replici-parinte : %d ", errno);
		exit(2);
	}
	
	char replica[4096];
	int cnt = 0;
	
	while(fgets(replica, 4096, F) != NULL) {
		
		// AFISARE REPLICA
		++cnt;
		printf("[R%d - parinte] %s", cnt, replica);
		
		if(*flag_addr == '2') {
			continue;
		}
		
		// YOUR TURN
		*flag_addr = '1';
		
		// WAIT FOR MY TURN
		do { 
			sleep(0); 
		} 
		while(*flag_addr == '1');
		sleep(2);
	}
	// Am terminat replicile setez flagul 2
	*flag_addr = '2';
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
	while(fgets(replica, 4096, F) != NULL) {
		
		if(*flag_addr == '2') {
			// AFISARE REPLICA 
			++cnt;
			printf("[R%d - Fiu] %s", cnt, replica);
			continue;
		}
		
		// WAIT FOR MY TURN

		do { 
			sleep(0); 
		} 
		while(*flag_addr == '0');
		
		// AFISARE REPLICA 
		++cnt;
		printf("[R%d - Fiu] %s", cnt, replica);
		
		// YOUR TURN
		*flag_addr = '0';
		sleep(2);
	}
	// Am terminat replicile setez flagul 2
	
	*flag_addr = '2';
	if(fclose(F) != 0) {
		fprintf(stderr, "Eroare la inchiderea fisierului replici_fiu : %d \n", errno);
		exit(3);
	} 
}


int main()
{
	pid_t pid_fiu;
	
	// Crearea unei mapari anonime in memorie
	flag_addr = mmap(NULL,
						 1,
						 PROT_READ | PROT_WRITE,
						 MAP_SHARED | MAP_ANONYMOUS,
						 -1,
						 0);
						 
	if(flag_addr == MAP_FAILED) {
		perror("Error at mmap");
		exit(5);
	}
	
	printf("Anonymous map of size 1 was created!");
	*flag_addr = '0';
	// initialization 
	
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
	if(-1 == munmap(flag_addr, 1)) {
		perror("Eroare a munmap");
		exit(5);
	}
	return 0;
}
