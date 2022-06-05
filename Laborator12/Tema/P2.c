#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#define PAGE_SIZE 4096

pid_t pid1, pid2;
sigset_t maskSon1, maskSon2, maskSon1With, allBlocked;

void handlerUSR1(int x) {
	printf("Transmitere SIGUSR1\n");
}
void handlerUSR2(int x) {
	printf("Transmitere SIGUSR2\n");
}

int main() {
	
	int fd;
	sigfillset(&maskSon1);
	sigfillset(&maskSon2);
	sigdelset(&maskSon1, SIGUSR2);
	sigdelset(&maskSon2, SIGUSR1);
	
	maskSon1With = maskSon1;
	sigdelset(&maskSon1With, SIGUSR1);
	// first son mask with sigusr1
	
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
		if(-1 == (pid2 = fork())) {
			perror("Eroare la fork");
			exit(1);
		}
		if(pid2) {
			// parinte
			if(-1 == (fd = open("linii.txt", O_WRONLY|O_TRUNC|O_CREAT, 0600))) {
				perror("Eroare la open");
				exit(2);
			}	
			srandom(getpid());
			int linii = random() % 15 + 1;
			// un numar random de linii de text intre 1 si 15
			for(int i = 0; i < linii; ++i) {
				int nChar = random() % 75 + 20;
				// un numar random de caractere din linie intre 20 si 75
				for(int j = 0; j < nChar; ++j) {
					char ch = random() % 80 + 40;
					if(j == nChar - 1) {
						ch = '\n';
					}
					if(-1 == write(fd, &ch, sizeof(char))) {
						perror("Eroare la scrierea in fisier\n");
						exit(3);
					}
				}
			}
			close(fd);
			
			sleep(1);
			if(-1 == kill(pid1, SIGUSR2)) {
				perror("Eroare la kill\n");
				exit(2);
			}
			if(-1 == kill(pid2, SIGUSR1)) {
				perror("Eroare la kill\n");
				exit(2);
			}
			wait(NULL);
			wait(NULL);
			
		} else {
			
			sigsuspend(&maskSon2);
			if(-1 == (fd = open("linii.txt", O_RDONLY))) {
				perror("Eroare la open");
				exit(2);
			}	
			char buffer[PAGE_SIZE];
			int noLines = 0;
			while(1) {
				int bytesRead = read(fd, buffer, PAGE_SIZE);
				if(bytesRead == -1) {
					perror("Eroare la read\n");
					exit(5);
				}
				if(!bytesRead) {
					break;
				}
				for(int i = 0; i < bytesRead; ++i) {
					if(buffer[i] == '\n') {
						noLines++;
					}
				}
			}
			close(fd);
			printf("Al doilea fiu a calculat un numar de linii de: %d\n", noLines);
			sleep(2);
			if(-1 == kill(pid1, SIGUSR1)) {
				perror("Eroare la kill\n");
				exit(3);
			}
			// Fiu 2
			exit(0);
		}
	} else {
		
		sigsuspend(&maskSon1);
		if(-1 == sigprocmask(SIG_BLOCK, &maskSon1, NULL)) {
			perror("Eroare la blocare");
			exit(5);
		}
		FILE *fi;
		if(NULL == (fi = fopen("linii.txt", "r"))) {
			perror("Eroare la open");
			exit(2);
		}	
		char line[256][PAGE_SIZE];
		int noLine = 0;
		while(NULL != fgets(line[noLine], PAGE_SIZE, fi)) {
			++noLine;
		}
		for(int i = 0; i < noLine; ++i) {
			for(int j = 0; j < noLine - 1; ++j) {
				if(strcmp(line[j], line[j + 1]) > 0) {
					for(int k = 0; k < PAGE_SIZE; ++k) {
						char c = line[j][k];
						line[j][k] = line[j + 1][k];
						line[j + 1][k] = c;
					}
				}
			}
		}
		sigsuspend(&maskSon1With);
		printf("Primul fiu a sortat vectorul de linii: \n");
		for(int i = 0; i < noLine; ++i) {
			printf("%s", line[i]);
		}
		fclose(fi);
		// Fiu 1
		exit(0);
	}
}
