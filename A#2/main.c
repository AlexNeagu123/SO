#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

int pipe01[2], pipe10[2], pipe02[2], pipe20[2];
int p1, p2;
FILE *fd;

void slaveWork1();
void slaveWork2();
void masterWork();

int main() {
	
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
		perror("Eroare la fork\n");
		exit(2);
	}

	if(NULL == (fd = fopen("rezultat.txt", "w"))) {
		perror("Eroare la open");
		exit(4);
	}
	
	if(p1 == 0) {
		
		slaveWork1();
		exit(0);
		
	} else {
		if(-1 == (p2 = fork())) {
			perror("Eroare la fork\n");
			exit(2);
		}
		if(p2 == 0) {
			
			slaveWork2();
			exit(0);
			
		} else {
			
			masterWork();
			exit(0);
		}
	}
}
void masterWork() {
	
	close(pipe01[0]);
	close(pipe10[1]);
	close(pipe02[0]);
	close(pipe20[1]);
	
	int x = -1;
	do {
		
		if(1 != scanf("%d", &x)) {
			printf("Nu ati introdus un numar valid\n");
			exit(2);
		}
		if(x != 0) {
			
			if(-1 == write(pipe01[1], &x, sizeof(int))) {
				perror("Eroare la write\n");
				exit(3);
			}
			
			char raspuns; 
			if(-1 == read(pipe10[0], &raspuns, 1)) {
				perror("Eroare la read\n");
				exit(4);
			}
			if(raspuns == '1') {
				fprintf(fd, "Numarul %d este prim.\n", x);
			} else {
			
				if(-1 == write(pipe02[1], &x, sizeof(int))) {
					perror("Eroare la write\n");
					exit(3);
				}
				
				int len;
				if(-1 == read(pipe20[0], &len, sizeof(int))) {
					perror("Eroare la read\n");
					exit(4);
				}
				int arr[len];
				if(-1 == read(pipe20[0], arr, sizeof(int) * len)) {
					perror("Eroare la read\n");
					exit(4);
				}
				fprintf(fd, "Numarul %d are divizorii: ", x);
				for(int i = 0; i < len; ++i) {
					fprintf(fd, "%d ", arr[i]);
				}
				fprintf(fd, "\n");
			}
		}
	} while(x != 0);
	
	close(pipe01[1]);
	close(pipe02[1]);
	close(pipe10[0]);
	close(pipe20[0]);
	fclose(fd);
}

void slaveWork1() {
	
	close(pipe01[1]);
	close(pipe10[0]);
	close(pipe02[0]);
	close(pipe02[1]);
	close(pipe20[0]);
	close(pipe20[1]);
	
	while(1) {
		
		int x;
		int bytesRead;
		if(-1 == (bytesRead = read(pipe01[0], &x, sizeof(int)))) {
			perror("Eroare la read\n");
			exit(4);
		}
		if(bytesRead == 0) {
			break;
		}
		
		char raspuns = '1';
		for(int i = 2; i * i <= x; ++i) {
			if(x % i == 0) {
				raspuns = '0';
			}
		}
		if(x == 1) raspuns = '0';
		if(-1 == write(pipe10[1], &raspuns, sizeof(char))) {
			perror("Eroare la write\n");
			exit(4);
		}
		
	}
	
	close(pipe01[0]);
	close(pipe10[1]);
	
}
void slaveWork2() {
	
	close(pipe01[0]);
	close(pipe01[1]);
	close(pipe10[0]);
	close(pipe10[1]);
	close(pipe02[1]);
	close(pipe20[0]);
	
	while(1) {
		
		int x;
		int bytesRead;
		if(-1 == (bytesRead = read(pipe02[0], &x, sizeof(int)))) {
			perror("Eroare la read\n");
			exit(4);
		}
		if(bytesRead == 0) {
			break;
		}
		
		int div[100], nr = 0;
		for(int i = 1; i <= x; ++i) {
			if(x % i == 0) {
				div[nr++] = i;
			}
		}
		
		if(-1 == write(pipe20[1], &nr, sizeof(int))) {
			perror("Eroare la write\n");
			exit(4);
		}
		if(-1 == write(pipe20[1], div, sizeof(int) * nr)) {
			perror("Eroare la write\n");
			exit(4);
		}
		
	}
	
	close(pipe02[0]);
	close(pipe20[1]);
}
