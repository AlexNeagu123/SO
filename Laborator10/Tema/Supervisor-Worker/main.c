#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>

#define BLOCK_SIZE 4096
#define handle_error(msg) \
	{ perror(msg); exit(EXIT_FAILURE); }


int main(int argc, char *argv[]) {
	
	 //argv[1] - numarul de workeri 
	 //argv[2] - nume file mare
	 //argv[3..] - optiuni
	  
	int N;
	if(sscanf(argv[1], "%d", &N) == 0) {
		fprintf(stderr, "Numarul de workeri introdus nu este valid");
		exit(1);
	}
	
	FILE *input;
	if(NULL == (input = fopen(argv[2], "rw"))) {
		handle_error("Eroare la deschiderea fisierului sursa text");
	}
	
	char files[N][256];
	for(int i = 0; i < N; ++i) {
		sprintf(files[i], "w%d.txt", i);
	}
	
	int fd[N];
	for(int i = 0; i < N; ++i) {
		if(-1 == (fd[i] = open(files[i], O_RDWR|O_TRUNC|O_CREAT, 0777))) {
			handle_error("Eroare la deschidere");
		}
	}
	
	
	int k = 0;
	char line[BLOCK_SIZE * 2];
	
	while(NULL != fgets(line, BLOCK_SIZE * 2, input)) {
		if(-1 == write(fd[k], line, strlen(line))) {
			handle_error("Eroare la scriere");
		}
		k = (k + 1) % N;
	}
	
	for(int i = 0; i < N; ++i) {
		if(-1 == close(fd[i])) {
			handle_error("Eroare la close");
		}
	}
	
	int pid;
	for(int i = 0; i < N; i++) {
		if(-1 == (pid = fork())) {
			perror("Eroare la fork");  
			exit(2);
		}
		if(0 == pid) {
			char *vec[argc];
			vec[0] = "myWc";
			for(int j = 3; j < argc; ++j) {
				vec[j - 2] = argv[j];
			}
			vec[argc - 2] = files[i];
			vec[argc - 1] = NULL;
			execv("myWc", vec);
			handle_error("Eroare la execv");
			exit(10);
		} 
	}
	
	int err = 0;
	int codTerm;
	
	for(int i = 0; i < N; ++i) {
		wait(&codTerm);
		if(WIFEXITED(codTerm)) {
			if(WEXITSTATUS(codTerm)) {
				err = WEXITSTATUS(codTerm);
			} 
		} 
	}
	
	if(err) {
		fprintf(stderr, "Programul nu s-a putut finaliza, codul de eroare: %d\n", err);
		exit(2);
	}
	
	unsigned long no_bytes = 0, no_words = 0;      // nr. de octeți, nr. de cuvinte
    unsigned int no_newlines = 0, max_length = 0;
    
	printf("Programul s-a executat cu succes\n");
	for(int i = 0; i < N; ++i) {
		
		int desc;
		if(-1 == (desc = open(files[i], O_RDONLY))) {
			handle_error("Eroare la open");
		}
		int *map_add = mmap(NULL,
						16,
						PROT_READ,
						MAP_PRIVATE,
						desc,
						0);
	
		if(map_add == MAP_FAILED) {
			handle_error("Eroare la mmap");
		}
		if(-1 == close(desc)) {
			handle_error("Eroare la close");
		}
		no_bytes += map_add[0];
		no_words += map_add[1];
		no_newlines += map_add[2];
		if(map_add[3] > max_length) {
			max_length = map_add[3];
		}
		if(-1 == munmap(map_add, 16)) {
			handle_error("Eroare la munmap");
		}
		
	}
	
	int fc = 0, fw = 0, fn = 0, fL = 0;
	for (int iterator = 1; iterator < argc; iterator++) {
        if(0 == strcmp(argv[iterator],"-c")) fc = 1;
        else if(0 == strcmp(argv[iterator],"-w")) fw = 1;
            else if(0 == strcmp(argv[iterator],"-l")) fn = 1;
                else if(0 == strcmp(argv[iterator],"-L")) fL = 1;
    }
	if (0 == fc + fw + fn + fL)   // Dacă nu-i specificată nici o opțiune în linia de comandă, le considerăm pe cele trei, similar comportamentului comenzii wc.
        fc = fw = fn = 1;

	printf("Fisierul %s contine: ", argv[2]);
	if(fc) printf("%lu caractere (octeti) , ", no_bytes);
	if(fw) printf("%lu cuvinte , ", no_words);
	if(fn) printf("%u linii de text (de fapt, newline-uri) , ", no_newlines);
	if(fL) printf("%u lungimea maxima a liniilor de text.", max_length);
	printf("\n");
	
	//for(int i = 0; i < N; ++i) {
		//if(-1 == remove(files[i])) {
			//handle_error("Eroare la stergerea fisierului");
		//}
	//}
}
