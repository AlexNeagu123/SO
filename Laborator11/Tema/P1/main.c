#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
int main() {
	//grep /bin/bash /etc/passwd | cut -d: -f1,3-5 | sort -t: -k 2,2 -n | cut -d, -f1
	int c12[2], c23[2], c34[2];
	pid_t p1, p2, p3;
	
	if(-1 == pipe(c34)) {
		perror("Eroare la pipe-ul 34");
		exit(1);
	}
	
	if(-1 == (p1 = fork())) {
		perror("Eroare la primul fork");
		exit(2);
	}
	
	if(p1 == 0) {
		if(-1 == pipe(c23)) {
			perror("Eroare la pipe-ul 23");
			exit(1);
		}
		if(-1 == (p2 = fork())) {
			perror("Eroare la al doilea fork");
			exit(2);
		}
		if(p2 == 0) {
			if(-1 == pipe(c12)) {
				perror("Eroare la pipe-ul 12");
				exit(1);
			}
			if(-1 == (p3 = fork())) {
				perror("Eroare la pipe-ul 12");
				exit(1);
			}
			if(p3 == 0) {
				if(-1 == dup2(c12[1], 1)) {
					perror("Eroare la dup2");
					exit(3);
				}
				execlp("grep", "grep", "/bin/bash", "/etc/passwd", NULL);
				perror("Eroare la execlp");
				exit(3); 
			} else {
				close(c12[1]);
				if(-1 == dup2(c12[0], 0)) {
					perror("Eroare la dup2");
					exit(3);
				}
				if(-1 == dup2(c23[1], 1)) {
					perror("Eroare la dup2");
					exit(3);
				}
				execlp("cut","cut","-d:","-f1,3-5", NULL);
				perror("Eroare la execlp");
				exit(3); 
			}
		} else {
			close(c23[1]);
			if(-1 == dup2(c23[0], 0)) {
				perror("Eroare la dup2");
				exit(3);
			}
			if(-1 == dup2(c34[1], 1)) {
				perror("Eroare la dup2");
				exit(3);
			}
			execlp("sort", "sort", "-t:", "-k", "2,2", "-n", NULL);
			perror("Eroare la execlp");
			exit(3); 
		}
	} else {
		close(c34[1]);
		if(-1 == dup2(c34[0], 0)) {
			perror("Eroare la dup2");
			exit(3);
		}
		execlp("cut", "cut", "-d,", "-f1", NULL);
		perror("Eroare la execlp");
		exit(3); 
	}
	return 0;
}
