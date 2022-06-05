#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


int pipe12[2], pipe21[2];
int p1, p2, fs;

void slaveWork1();
void slaveWork2();

int main() {
	// parintele - 0, fiul1 - 1, fiul2 - 2
	if(-1 == (fs = open("Persoane.txt", O_WRONLY|O_TRUNC|O_CREAT))) {
		perror("Eroare la open");
		exit(4);
	}
	
	if(-1 == pipe(pipe12)) {
		perror("Eroare la pipe\n");
		exit(1);
	}
	if(-1 == pipe(pipe21)) {
		perror("Eroare la pipe\n");
		exit(1);
	}
	if(-1 == (p1 = fork())) {
		perror("Eroare la fork\n");
		exit(3);
	}
	if(p1 == 0) {
		slaveWork1();
		exit(0);
	} else {
		if(-1 == (p2 = fork())) {
			perror("Eroare la fork\n");
			exit(3);
		}
		if(p2 == 0) {
			slaveWork2();
			exit(0);
		} else {
			
			sleep(1);
			close(pipe12[0]);
			close(pipe12[1]);
			close(pipe21[0]);
			close(pipe21[1]);
			wait(NULL);
			wait(NULL);
			close(fs);
			exit(0);
			
		}
	}
	
}

void slaveWork1() {
	
	FILE *fd;
	
	if(NULL == (fd = fopen("Nume.txt", "r"))) {
		perror("Eroare la open");
		exit(4);
	}
	
	char flag;
	int eoNume = 0;
	
	close(pipe12[0]);
	close(pipe21[1]);
	
	while(1) {
		// CITIRE NUME
		char line[256];
		if(NULL == fgets(line, sizeof(line), fd)) {
			eoNume = 1;
		}
		// Scriere In Persoane
		printf("Valoarea flagului eoNume: %d\n", eoNume);
		if(!eoNume) {
			if(-1 == write(fs, line, strlen(line))) {
				perror("Eroare la write\n");
				exit(4);
			}
		}
		// E randul tau 
		char ch;
		if(eoNume == 1) {
			ch = '2';
			if(-1 == write(pipe12[1], &ch, sizeof(char))) {
				perror("Eroare la write\n");
				exit(4);
			}
			close(pipe12[1]);
			break;
		} else {
			ch = '1';
			if(-1 == write(pipe12[1], &ch, sizeof(char))) {
				perror("Eroare la write\n");
				exit(4);
			}
		}
		
		// ASTEPT RANDUL
		if(-1 == read(pipe21[0], &flag, sizeof(flag))) {
			perror("Eroare la read");
			exit(4);
		}
		if(flag == '2') {
			break;
		}
	}
	
	close(pipe21[0]);
	if(flag == '2') {
		printf("Fiu1 - s-au epuizat prenumele din fiu2\n");
	}
	
	fclose(fd);
	close(fs);

}
void slaveWork2() {
	
	FILE *fd;
	if(NULL == (fd = fopen("Prenume.txt", "r"))) {
		perror("Eroare la open");
		exit(4);
	}
	
	
	char flag;
	int eoPrenume = 0;
	
	close(pipe21[0]);
	close(pipe12[1]);
	
	while(1) {
		// ASTEPT RANDUL MEU
		if(-1 == read(pipe12[0], &flag, sizeof(flag))) {
			perror("Eroare la read");
			exit(4);
		}
		if(flag == '2') {
			break;
		}
		// Citesc si Actualizez
		char line[256];
		if(NULL == fgets(line, sizeof(line), fd)) {
			eoPrenume = 1;
		}
		printf("Fiu2 - a citit textul %s\n", line);
		if(!eoPrenume) {
			if(-1 == write(fs, line, strlen(line))) {
				perror("Eroare la write\n");
				exit(4);
			}
		}
		// E randul tau 
		char ch;
		if(eoPrenume == 1) {
			ch = '2';
			if(-1 == write(pipe21[1], &ch, sizeof(char))) {
				perror("Eroare la write\n");
				exit(4);
			}
			close(pipe21[1]);
			break;
		} else {
			ch = '1';
			if(-1 == write(pipe21[1], &ch, sizeof(char))) {
				perror("Eroare la write\n");
				exit(4);
			}
		}
	}
	
	close(pipe12[0]);
	if(flag == '2') {
		printf("Fiu2 - s-au epuizat numele din Fiu1\n");
	}
	
	fclose(fd);
	close(fs);
}
