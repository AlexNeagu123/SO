#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#define PAGE 4096

int pipe01[2], pipe02[2], pipe10[2], pipe20[2];
char firstFile[PAGE], secondFile[PAGE];
int p1, p2;

void slaveWork1();
void slaveWork2();
void masterWork();

int main() {
	
	printf("Introduceti numele primului fisier: ");
	if(1 != scanf("%s", firstFile)) {
		perror("Eroare la citire");
		exit(2);
	}
	printf("Introduceti numele celui de-al doilea fisier: ");
	if(1 != scanf("%s", secondFile)) {
		perror("Eroare la citire");
		exit(2);
	}

	
	if(-1 == pipe(pipe01)) {
		perror("Eroare la pipe\n");
		exit(1);
	}
	if(-1 == pipe(pipe02)) {
		perror("Eroare la pipe\n");
		exit(1);
	}
	if(-1 == pipe(pipe10)) {
		perror("Eroare la pipe\n");
		exit(1);
	}
	if(-1 == pipe(pipe20)) {
		perror("Eroare la pipe\n");
		exit(1);
	}
	if(-1 == (p1 = fork())) {
		perror("Eroare la fork");
		exit(2);
	}
	if(p1 == 0) {
		
		slaveWork1();
		exit(0);
		
	} else {
		if(-1 == (p2 = fork())) {
			perror("Eroare la fork");
			exit(2);
		}
		if(p2 == 0) {
			
			slaveWork2();
			exit(0);
			
		} else {
			
			masterWork();
			
		}
	}
}

void masterWork() {
	
	sleep(1);
	close(pipe01[0]);
	close(pipe02[0]);
	close(pipe10[1]);
	close(pipe20[1]);
	if(-1 == write(pipe01[1], firstFile, strlen(firstFile))) {
		perror("Eroare la write");
		exit(3);
	}
	if(-1 == write(pipe02[1], secondFile, strlen(secondFile))) {
		perror("Eroare la write");
		exit(3);
	}
	
	close(pipe01[1]);
	close(pipe02[1]);
	
	wait(NULL);
	wait(NULL);
	
	int nBytes1 = 0;
	while(1) {
		
		char ch;
		int bytesRead;
		if(-1 == (bytesRead = read(pipe10[0], &ch, 1))) {
			perror("Eroare la read\n");
			exit(5);
		}
		if(bytesRead == 0) {
			break;
		}
		nBytes1 += bytesRead;	
	}
	close(pipe10[0]);
	
	int nBytes2 = 0;
	while(1) {
		
		char ch;
		int bytesRead;
		if(-1 == (bytesRead = read(pipe20[0], &ch, 1))) {
			perror("Eroare la read\n");
			exit(5);
		}
		if(bytesRead == 0) {
			break;
		}
		nBytes2 += bytesRead;
	}
	close(pipe20[0]);
	
	printf("Numarul de octeti trimisi de fiul1: %d\n", nBytes1);
	printf("Numarul de octeti trimisi de fiul2: %d\n", nBytes2);
	printf("Concluzie: ");
	if(nBytes1 > nBytes2) {
		printf("Primul fiu a trimis mai multe caractere\n");
	} else if(nBytes1 == nBytes2) {
		printf("Ambii fii au trimis la fel de multe carectere\n");
	} else {
		printf("Al doilea fiu a trimis mai multe caractere\n");
	}
}
void slaveWork1() {
	
	close(pipe01[1]);
	close(pipe10[0]);
	close(pipe02[0]);
	close(pipe02[1]);
	close(pipe20[0]);
	close(pipe20[1]);
	
	char fileName[256];
	int p = 0;
	while(1) {
		int bytesRead;
		char ch;
		if(-1 == (bytesRead = read(pipe01[0], &ch, 1))) {
			perror("Eroare la read\n");
			exit(4);
		}	
		if(bytesRead == 0) {
			break;
		}
		fileName[p++] = ch;
	}
	fileName[p] = '\0';
	close(pipe01[0]);
	
	if(-1 == dup2(pipe10[1], 1)) {
		perror("Eroare la dup");
		exit(5);
	}
	
	execlp("grep", "grep", "pipe", fileName, NULL);
	perror("Eroare la execlp");
	exit(6);
}

void slaveWork2() {
	
	close(pipe02[1]);
	close(pipe20[0]);
	close(pipe01[0]);
	close(pipe01[1]);
	close(pipe10[0]);
	close(pipe10[1]);
	
	char fileName[256];
	int p = 0;
	while(1) {
		int bytesRead;
		char ch;
		if(-1 == (bytesRead = read(pipe02[0], &ch, 1))) {
			perror("Eroare la read\n");
			exit(4);
		}	
		if(bytesRead == 0) {
			break;
		}
		fileName[p++] = ch;
	}
	
	fileName[p] = '\0';
	close(pipe02[0]);
	
	printf("fisierul primit de w2: %s\n", fileName);
	close(pipe02[0]);
	
	if(-1 == dup2(pipe20[1], 1)) {
		perror("Eroare la dup");
		exit(5);
	}
	
	execlp("grep", "grep", "pipe", fileName, NULL);
	perror("Eroare la execlp");
	exit(6);
	
}
