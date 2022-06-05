#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
void rec(int i, int j) {
	// i - nivelul procesului
	// j - numarul procesului de pe nivel
	int codterm;
	
	if(i == 1) {
		
		pid_t fiu1, fiu2;
		if(-1 == (fiu1 = fork())) {
			perror("Eroare la fork!\n");
			exit(1);
		}
		
		if(fiu1 == 0) {
			rec(2, 1);
		} else {
			
			int codterm;
			wait(&codterm);
			printf("Sunt procesul %d%d, avand PID-ul: %d, parintele are PID-ul: %d, iar fiul creat are PID-ul: %d si s-a terminat cu codul: %d.\n", i, j, getpid(), getppid(), fiu1, codterm >> 8 );
			if(-1 == (fiu2 = fork())) {
				perror("Eroare la fork!\n");
				exit(1);
			}
			if(fiu2 == 0) {
				rec(2, 2);
			} else {
				int codterm;
				wait(&codterm);
				printf("Sunt procesul %d%d, avand PID-ul: %d, parintele are PID-ul: %d, iar fiul creat are PID-ul: %d si s-a terminat cu codul: %d.\n", i, j, getpid(), getppid(), fiu2, codterm >> 8 );
				exit(1);
			}
		}
		
	}
	
	if(i == 2) {
		
		int add = 0;
		if(j == 2) add = 3;
		
		pid_t fiu1, fiu2, fiu3;
		if(-1 == (fiu1 = fork())) {
			perror("Eroare la fork!\n");
			exit(1);
		}
		if(fiu1 == 0) {
			rec(3, 1 + add);
		} else {
			wait(&codterm);
			printf("Sunt procesul %d%d, avand PID-ul: %d, parintele are PID-ul: %d, iar fiul creat are PID-ul: %d si s-a terminat cu codul: %d.\n", i, j, getpid(), getppid(), fiu1, codterm >> 8 );
			if(-1 == (fiu2 = fork())) {
				perror("Eroare la fork!\n");
				exit(1);
			}
			if(fiu2 == 0) {
				rec(3, 2 + add);
			} else {
				wait(&codterm);
				printf("Sunt procesul %d%d, avand PID-ul: %d, parintele are PID-ul: %d, iar fiul creat are PID-ul: %d si s-a terminat cu codul: %d.\n", i, j, getpid(), getppid(), fiu2, codterm >> 8 );
				if(-1 == (fiu3 = fork())) {
					perror("Eroare la fork!\n");
					exit(1);
				}
				if(fiu3 == 0) {
					rec(3, 3 + add);
				} else {
					wait(&codterm);
					printf("Sunt procesul %d%d, avand PID-ul: %d, parintele are PID-ul: %d, iar fiul creat are PID-ul: %d si s-a terminat cu codul: %d.\n", i, j, getpid(), getppid(), fiu3, codterm >> 8 );
					exit(2);
				}
				
			}
		}
	}
	if(i == 3) {
		printf("Sunt procesul ultim %d%d, avand PID-ul: %d, parintele are PID-ul: %d.\n", i, j, getpid(), getppid());
		exit(3);
	}
	
	
}
int main() {
	rec(1, 1);
}
