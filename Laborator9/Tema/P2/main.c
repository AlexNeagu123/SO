#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

int N = 0;

void calcul_tata()
{
	//TODO: de implementat calculul efectuat de către supervisor, conform indicațiilor cuprinse în diagramă!
	int fd, fs;
	
	if(-1 == (fd = open("operanzi.bin", O_RDONLY))) {
		perror("Eroare la deschiderea fisierului operanzi");
		exit(1);
	}
	if(-1 == (fs = open("operatori.txt", O_RDONLY))) {
		perror("Eroare la deschiderea fisierului operatori");
		exit(1);
	}
	
	struct stat st_fd;
	if(stat("operanzi.bin", &st_fd) == -1) {
		perror("Eroare la stat");
		exit(3);
	}
	struct stat st_fs;
	if(stat("operatori.txt", &st_fs) == -1) {
		perror("Eroare la stat");
		exit(3);
	}
	
	int *nr_address = mmap(NULL,
						   st_fd.st_size,
						   PROT_READ,
						   MAP_PRIVATE, 
						   fd, 
						   0);
	if(nr_address == MAP_FAILED) {
		perror("Error at mmap");
		exit(5);
	}
	char *op_address = mmap(NULL, 
						   st_fs.st_size,
						   PROT_READ,
						   MAP_PRIVATE,
						   fs,
						   0);
	if(op_address == MAP_FAILED) {
		perror("Error at mmap");
		exit(5);
	}
	if(-1 == close(fd)) {
		perror("Eroare la inchiderea fisierului operanzi!");
		exit(2);
	}
	if(-1 == close(fs)) {
		perror("Eroare la inchiderea fisierului operatori!");
		exit(2);
	}
	
	for(int i = 0; i < N; ++i) {
		char op;
		op = *(op_address + i);
		int x, y;
		x = *(nr_address + 2 * i);
		y = *(nr_address + 2 * i + 1);
		printf("[Operatia %d] Se realizeaza operatia %c pe numerele %d si %d:\n", i, op, x, y);
		double answer;
		
		if(op == '+') {
			answer = x + y;
		}
		if(op == '-') {
			answer = x - y;
		}
		if(op == '*') {
			answer = x * y;
		}
		if(op == '/') {
			answer = (float) x / y;
		}
		
		printf("[Operatia %d] Rezultatul operatiei este %f\n", i, answer);
	}
	
	if(munmap(nr_address, st_fd.st_size) == -1) {
		perror("Eroare la stergerea maparii");
		exit(5);
	}
	if(munmap(op_address, st_fs.st_size) == -1) {
		perror("Eroare la stergerea maparii");
		exit(5);
	}
	
	if(-1 == remove("operatori.txt")) {
		perror("Eroare la stergerea fisierului operatori.txt");
		exit(4);
	}
	if(-1 == remove("operanzi.bin")) {
		perror("Eroare la stergerea fisierului operanzi.bin");
		exit(4);
	}
}

void generare_fiu1()
{
	// TODO: de implementat generarea operanzilor de către workerul #1, conform indicațiilor cuprinse în diagramă!
	int fd;
	if(-1 == (fd = open("operanzi.bin", O_RDWR | O_CREAT, 0777))) {
		perror("Eroare la deschiderea fisierului operanzi");
		exit(1);
	}
	if(-1 == ftruncate(fd, 2 * sizeof(int) * N)) {
		perror("Eroare la trunchiere");
		exit(5);
	}
	
	int *map_addr = mmap(NULL,
						 2 * sizeof(int) * N, 
						 PROT_READ | PROT_WRITE,
						 MAP_SHARED,
						 fd, 
						 0);
						
	if(map_addr == MAP_FAILED) {
		perror("Eroare la map!");
		exit(4);
	}
	if(-1 == close(fd)) {
		perror("Eroare la inchiderea fisierului operanzi!");
		exit(2);
	}
	
	srandom(getpid()); // generatorul de numere aleatoare
	for(int i = 0; i < N; ++i) {
		int x = random() % 1000;
		int y = random() % 1000;
		*(map_addr + 2 * i) = x;
		*(map_addr + 2 * i + 1) = y;
	}
	
	if(-1 == msync(map_addr, 8 * N, MS_SYNC)) {
		perror("Eroare la msync");
		exit(6);
	}
	if(-1 == munmap(map_addr, 8 * N)) {
		perror("Eroare la munmap");
		exit(6);
	}
}

void generare_fiu2()
{
	// TODO: de implementat generarea operatorilor de către workerul #2, conform indicațiilor cuprinse în diagramă!
	int fd;
	if(-1 == (fd = open("operatori.txt", O_RDWR | O_CREAT | O_TRUNC, 0700))) {
		perror("Eroare la deschiderea fisierului operatori");
		exit(1);
	}
	
	if(-1 == ftruncate(fd, sizeof(char) * N)) {
		perror("Eroare la trunchiere");
		exit(5);
	}
	
	char *map_addr = mmap(NULL,
						 sizeof(char) * N, 
						 PROT_READ | PROT_WRITE,
						 MAP_SHARED,
						 fd, 
						 0);
	
	if(map_addr == MAP_FAILED) {
		perror("Eroare la map!");
		exit(4);
	}
	if(-1 == close(fd)) {
		perror("Eroare la inchiderea fisierului operanzi!");
		exit(2);
	}
	
	srandom(getpid()); // generatorul de numere aleatoare
	for(int i = 0; i < N; ++i) {
		int op = random() % 4;
		char buff;
		switch (op) {
			case 0 : 
				buff = '+';
				break;
			case 1 :
				buff = '-';
				break;
			case 2 : 
				buff = '*';
				break;
			case 3 :
				buff = '/';
				break;
		}
		*(map_addr + i) = buff;
	}
	
	if(-1 == msync(map_addr, sizeof(char) * N, MS_SYNC)) {
		perror("Eroare la msync");
		exit(6);
	}
	if(-1 == munmap(map_addr, sizeof(char) * N)) {
		perror("Eroare la munmap");
		exit(6);
	}
}


int main()
{
	pid_t pid_fiu1, pid_fiu2;

	printf("Dati numarul intreg N:");
	if(1 != scanf("%d", &N)) {
		fprintf(stderr, "Eroare! Valoarea introdusa nu este numar natural");
		exit(0);
	} // TODO: tratați excepțiile de citire a unui număr întreg
	

	/* Crearea procesului fiu #1. */
	if(-1 == (pid_fiu1 = fork()) )
	{
		perror("Eroare la fork #1");  
		return 1;
	}

	/* Ramificarea execuției după primul apel fork. */
	if(pid_fiu1 == 0)
	{   /* Zona de cod executată doar de către fiul #1. */
		printf("\n[P1] Procesul fiu 1, cu PID-ul: %d.\n", getpid());
		generare_fiu1();
		return 0;
	}
	else
	{   /* Zona de cod executată doar de către părinte. */
		printf("\n[P0] Procesul tata, cu PID-ul: %d.\n", getpid());

		/* Crearea procesului fiu #2. */
		if(-1 == (pid_fiu2 = fork()) )
		{
			perror("Eroare la fork #2");  
			return 2;
		}
		
		/* Ramificarea execuției după al doilea apel fork. */
		if(pid_fiu2 == 0)
		{   /* Zona de cod executată doar de către fiul #2. */
			printf("\n[P2] Procesul fiu 2, cu PID-ul: %d.\n", getpid());
			generare_fiu2();
			return 0;
		}
		else
		{   /* Zona de cod executată doar de către părinte. */

			wait(NULL);
			wait(NULL);
			// Mai întâi aștept terminarea ambilor fii, iar apoi fac calculul.
			calcul_tata();
			return 0;
		}
	}
}
