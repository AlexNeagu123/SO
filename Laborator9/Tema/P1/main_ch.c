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
			printf("%d-%d\n", getpid(), getppid());
			if(-1 == (fiu2 = fork())) {
				perror("Eroare la fork!\n");
				exit(1);
			}
			if(fiu2 == 0) {
				rec(2, 2);
			} else {
				int codterm;
				wait(&codterm);
				printf("%d-%d\n", getpid(), getppid());				
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
			printf("%d-%d\n", getpid(), getppid());			
			if(-1 == (fiu2 = fork())) {
				perror("Eroare la fork!\n");
				exit(1);
			}
			if(fiu2 == 0) {
				rec(3, 2 + add);
			} else {
				wait(&codterm);
				printf("%d-%d\n", getpid(), getppid());			
				if(-1 == (fiu3 = fork())) {
					perror("Eroare la fork!\n");
					exit(1);
				}
				if(fiu3 == 0) {
					rec(3, 3 + add);
				} else {
					wait(&codterm);
					printf("%d-%d\n", getpid(), getppid());			
					exit(2);
				}
				
			}
		}
	}
	if(i == 3) {
		printf("%d-%d\n", getpid(), getppid());			
		exit(3);
	}
	
	
}
int main() {
	rec(1, 1);
}
