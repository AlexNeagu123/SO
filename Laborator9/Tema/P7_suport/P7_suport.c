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
	
	for(int i = 0; i < N; ++i) {
		char op;
		if(-1 == read(fs, &op, 1)) {
			perror("Eroare la citirea din fisierul operatii");
			exit(3);
		}
		int x, y;
		if(-1 == read(fd, &x, sizeof(int))) {
			perror("Eroare la citirea din fisierul operanzi");
			exit(3);
		}
		if(-1 == read(fd, &y, sizeof(int))) {
			perror("Eroare la citirea din fisierul operanzi");
			exit(3);
		}
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
	
	if(-1 == close(fd)) {
		perror("Eroare la inchiderea fisierului operanzi!");
		exit(2);
	}
	if(-1 == close(fs)) {
		perror("Eroare la inchiderea fisierului operatori!");
		exit(2);
	}
	//if(-1 == remove("operatori.txt")) {
		//perror("Eroare la stergerea fisierului operatori.txt");
		//exit(4);
	//}
	//if(-1 == remove("operanzi.bin")) {
		//perror("Eroare la stergerea fisierului operanzi.bin");
		//exit(4);
	//}
}

void generare_fiu1()
{
	// TODO: de implementat generarea operanzilor de către workerul #1, conform indicațiilor cuprinse în diagramă!
	int fd;
	if(-1 == (fd = open("operanzi.bin", O_WRONLY | O_CREAT | O_TRUNC, 0600))) {
		perror("Eroare la deschiderea fisierului operanzi");
		exit(1);
	}
	srandom(getpid()); // generatorul de numere aleatoare
	for(int i = 0; i < N; ++i) {
		int x = random() % 1000;
		int y = random() % 1000;
		if(-1 == write(fd, &x, 4)) {
			perror("Eroare la scriere in fisierul operanzi!");
			exit(2);
		}
		if(-1 == write(fd, &y, 4)) {
			perror("Eroare la scriere in fisierul operanzi!");
			exit(2);
		}
	}
	if(-1 == close(fd)) {
		perror("Eroare la inchiderea fisierului operanzi!");
		exit(2);
	}
}

void generare_fiu2()
{
	// TODO: de implementat generarea operatorilor de către workerul #2, conform indicațiilor cuprinse în diagramă!
	int fd;
	if(-1 == (fd = open("operatori.txt", O_WRONLY | O_CREAT | O_TRUNC, 0700))) {
		perror("Eroare la deschiderea fisierului operatori");
		exit(1);
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
		if(-1 == write(fd, &buff, 1)) {
			perror("Eroare la scriere in fisierul operatori!");
			exit(2);
		}
	}
	if(-1 == close(fd)) {
		perror("Eroare la inchiderea fisierului operatori!");
		exit(2);
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
