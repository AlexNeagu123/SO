#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define PAGE 4096
char buffer[PAGE];
int isSmall(char c) {
	return (c >= 'a' && c <= 'z');
}
int main() {
	
	pid_t p1, p2;
	int c12[2], c23[2], c31[2];
	
	if(-1 == pipe(c12)) {
		perror("[P0] Eroare la pipe");
	}
	if(-1 == pipe(c23)) {
		perror("[P0] Eroare la pipe");
	}
	if(-1 == pipe(c31)) {
		perror("[P0] Eroare la pipe");
	}
	
	if(-1 == (p1 = fork())) {
		perror("[P0]Eroare la fork");
		exit(1);
	}
	
	if(p1 == 0) {
		close(c12[1]);
		close(c23[0]);
		close(c31[0]);
		close(c31[1]);
		while(1) {
			int bytesRead;
			if(-1 == (bytesRead = read(c12[0], buffer, PAGE))) {
				perror("[P1]Eroare la read");
				exit(2);
			}
			if(bytesRead == 0) {
				break;
			}
			char newBuff[PAGE];
			int c = 0;
			for(int i = 0; i < bytesRead; ++i) {
				if(isSmall(buffer[i])) {
					newBuff[c++] = buffer[i];
				}
			}
			if(-1 == write(c23[1], newBuff, c)) {
				perror("[P1]Eroare la write");
				exit(2);
			}
		}
		exit(0);
	} else {
		
		if(-1 == (p2 = fork())) {
			perror("[P2]Eroare la fork");
			exit(1);
		}
		
		if(p2 == 0) {
			
			close(c23[1]);
			close(c12[0]);
			close(c12[1]);
			close(c31[0]);
			
			int desc;
			if(-1 == (desc = open("statistica.txt", O_WRONLY|O_CREAT|O_TRUNC, 0600))) {
				perror("[P2]Eroare la crearea fisierului statistica.txt: ");
				exit(4);
			}
			
			int f[30];
			for(int i = 0; i < 30; ++i) {
				f[i] = 0;
			}
			
			while(1) {
				int bytesRead;
				if(-1 == (bytesRead = read(c23[0], buffer, PAGE))) {
					perror("[P2]Eroare la read");
					exit(2);
				}
				if(bytesRead == 0) {
					break;
				}
				for(int i = 0; i < bytesRead; ++i) {
					f[buffer[i] - 'a']++;
				}
			}
			
			int nr = 0;
			for(int i = 0; i < 30; ++i) {
				if(f[i]) {
					++nr;
					char add = i + 'a';
					char ch[256];
					int bRead = snprintf(ch, sizeof(ch), "%c %d\n", add, f[i]);
					if(-1 == write(desc, ch, bRead)) {
						perror("[P2]Eroare la write");
						exit(2);
					}
				}
			}
			
			write(c31[1], &nr, sizeof(int));
			if(-1 == close(desc)) {
				perror("[P2]Eroare la inchiderea fisierului statistica");
				exit(5);
			}
			exit(0);
		}
		else {
			
			close(c12[0]);
			close(c31[1]);
			close(c23[0]);
			close(c23[1]);
			
			int fd;
			if(-1 == (fd = open("date.txt", O_RDONLY))) {
				perror("[P0] Eroare la open");
				exit(1);
			}
			
			while(1) {
				int bytesRead;
				if(-1 == (bytesRead = read(fd, buffer, PAGE))) {
					perror("[P0] Eroare la read");
					exit(2);
				}
				if(0 == bytesRead) {
					break;
				}
				if(-1 == write(c12[1], buffer, PAGE)) {
					perror("[P0] Eroare la write");
					exit(2);
				}
			}
			close(c12[1]);
			
			if(-1 == close(fd)) {
				perror("[P0] Eroare la close");
				exit(5);
			} 
			int x;
			if(-1 == read(c31[0], &x, 4)) {
				perror("Eroare la read");
				exit(2);
			}
			printf("[P0] Numarul de litere mici din date.txt este: %d\n", x);
		}
	}
}
