#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

sigset_t sonMask;
pid_t pid1;
// variabila globala B=1 daca procesul fiu mai este in viata 

void handlerSigInt(int id) {
	printf("In procesul %d q fost generat semnalul %d\n", getpid(), id);
	printf("I se da procesului fiu semnalul SIGUSR2\n");
	if(-1 == kill(pid1, SIGUSR2)) {
		perror("Eroare la kill\n");
		exit(2);
	}
	wait(NULL);
	printf("Procesul fiu s-a terminat, deci si parintele se termina\n");
	exit(0);
}

void handlerUSR1(int id) {
	//printf("In procesul %d q fost generat semnalul %d\n", getpid(), id);
	printf("Fiul: am primit USR1\n");
}
void handlerUSR2(int id) {
	printf("Fiul: sfarsit executie\n");
	exit(0);
}

int main() {
	
	sigfillset(&sonMask);
	sigdelset(&sonMask, SIGUSR1);
	sigdelset(&sonMask, SIGUSR2);
	
	if(SIG_ERR == signal(SIGUSR1, handlerUSR1)) {
		perror("Eroare la signal\n");
		exit(3);
	}
	
	if(SIG_ERR == signal(SIGUSR2, handlerUSR2)) {
		perror("Eroare la signal\n");
		exit(3);
	}
		
	if(-1 == (pid1 = fork())) {
		perror("Eroare la fork");
		exit(1);
	}
	
	if(pid1) {
		// parinte
		if(SIG_ERR == signal(SIGINT, handlerSigInt)) {
			perror("Eroare la signal\n");
			exit(3);
		}
		int count = 0;
		while(1) {
			printf("%d\n", count);
			count++;
			sleep(1);
			if(count % 10 == 0) {
				if(-1 == kill(pid1, SIGUSR1)) {
					perror("Eroare la kill\n");
					exit(2);
				}
			}
		}
	} else {
		while(1) {
			sigsuspend(&sonMask);
		}
	}
	
}
