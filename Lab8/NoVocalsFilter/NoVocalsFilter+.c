#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#define BLOCK_SIZE 4096;
#define handle_error(msg) \
    { perror(msg); exit(EXIT_FAILURE); }

int ask_suprascrie(char *dest);
void general_case(int argc, char *argv[]);
void special_case(int argc, char *argv[]);
int main(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Este nevoie cel putin un fisier dat prin parametru (source -> destination) sau source -> source (trunc) \n./ ");
		exit(1);
	}
	if(argc == 2 || !strcmp(argv[1], argv[2])) {
		special_case(argc, argv);
	} else if(argc == 3) {
		general_case(argc, argv);
	}
}
void special_case(int argc, char *argv[]) {
	int flag_suprascrie = ask_suprascrie(argv[1]);
	if(!flag_suprascrie) {
		fprintf(stderr, "Copierea nu a avut loc - utilizatorul nu a fost de acord sa suprascrie destinatia\n");
		exit(4);
	}
	printf("Utilizatorul a acceptat sa suprascrie destinatia\n");
	int fs = open(argv[1], O_RDWR);
	if(fs == -1) {
		handle_error("Eroare la deschiderea fisierului sursa");
		exit(2);
	}
	long PageSize = sysconf(_SC_PAGE_SIZE);
	if (PageSize == -1) {
		handle_error("Error at sysconf");
	}
	fprintf(stderr, "Notification: the page size on your hardware system is: %ld bytes.\n", PageSize);
	// I NEED THE FILE SIZE!
	struct stat sb;
	if (fstat(fs, &sb) == -1) {  /* To obtain file size */
		handle_error("Error at fstat");
	}
	char *map_addr = mmap(NULL, // address of mapping
						sb.st_size, // size of the source file will be the length
						PROT_READ|PROT_WRITE, // Doar pentru citire
						MAP_SHARED, // PROT_READ merge cu MAP_PRIVATE
						fs, // fisierul mapat
						0); // incepem sa mapam de la inceputul fisierului (este multiplu de dim paginii);
	if (map_addr == MAP_FAILED) {
		handle_error("Error at mmap");
	}
	fprintf(stderr, "Notification: A file mapping in memory was created, for the source+dest file\n");
	
	int len = sb.st_size;
	while(1) {
		int found = 0;
		for(int i = 0; i < len && !found; ++i) {
			if(strchr("aeiouAEIOU", *(map_addr + i))) {
				found = 1;
				for(int j = i; j < len - 1; ++j) {
					*(map_addr + j) = *(map_addr + j + 1);
				}
				len--;
			}
		}
		if(!found) {
			break;
		}
	}
	if(-1 == ftruncate(fs, len)) {
		handle_error("Eroare la trunchiere");
	}
	if (-1 == close(fs)) {
		handle_error("Error at close");
	}
	if (-1 == msync(map_addr, len, MS_SYNC)) {
		handle_error("Error at msync");
	}
	if (-1 == munmap(map_addr, len)) {
		handle_error("Error at munmap");
	}
	
}
void general_case(int argc, char *argv[]) {
	int fs = open(argv[1], O_RDONLY), fd;
	if(fs == -1) {
		handle_error("Eroare la deschiderea fisierului sursa");
		exit(2);
	}
	int exists = access(argv[2], F_OK);
	if(exists == -1) {
		fd = open(argv[2], O_CREAT|O_RDWR, 0600);
		if(fd == -1) {
			handle_error("Eroare la deschiderea fisierului destinatie: ");
			exit(3);
		}
	} else {
		int flag_suprascrie = ask_suprascrie(argv[2]);
		if(!flag_suprascrie) {
			fprintf(stderr, "Copierea nu a avut loc - utilizatorul nu a fost de acord sa suprascrie destinatia\n");
			exit(4);
		}
		printf("Utilizatorul a acceptat sa suprascrie destinatia\n");
		fd = open(argv[2], O_RDWR);
		if(fd == -1) {
			handle_error("Eroare la deschiderea fisierului destinatie: ");
			exit(3);
		}
	}
//  -----------------------------------MAPPING PART FOR READING (stored in string buffer)----------------------------------------------------------
	long PageSize = sysconf(_SC_PAGE_SIZE);
	if (PageSize == -1) {
		handle_error("Error at sysconf");
	}
	fprintf(stderr, "Notification: the page size on your hardware system is: %ld bytes.\n", PageSize);
	// I NEED THE FILE SIZE!
	struct stat sb;
	if (fstat(fs, &sb) == -1) {  /* To obtain file size */
		handle_error("Error at fstat");
	}
	char *map_fs_addr = mmap (NULL, // address of mapping
						sb.st_size, // size of the source file will be the length
						PROT_READ, // Doar pentru citire
						MAP_PRIVATE, // PROT_READ merge cu MAP_PRIVATE
						fs, // fisierul mapat
						0); // incepem sa mapam de la inceputul fisierului (este multiplu de dim paginii)
	if (map_fs_addr == MAP_FAILED) {
		handle_error("Error at mmap");
	}
	fprintf(stderr, "Notification: A file mapping in memory was created, for the source file\n");
	if (-1 == close(fs)) {
		handle_error("Error at close");
	}
	char buffer[PageSize * 4];
	char *pBuffer = buffer;
	for(int i = 0; i < sb.st_size; ++i) {
		if(!strchr("aeiouAEIOU", *(map_fs_addr + i))) {
			*pBuffer = *(map_fs_addr + i);
			pBuffer++;
		}
	}
	*pBuffer = '\0';
	// stergem maparea din memorie pentru read (nu e neaparat sa sincronizez aici)
	if (-1 == munmap(map_fs_addr, sb.st_size)) {
		handle_error("Error at munmap");
	}
//  -----------------------------------MAPPING PART FOR WRITING (from string buffer to file fd)----------------------------------------------------------
	int length = strlen(buffer);
	ftruncate(fd, length);
	char *map_fd_addr = mmap (NULL,
							  length, 
							  PROT_READ|PROT_WRITE,
							  MAP_SHARED,
							  fd,
							  0);
	if (map_fd_addr == MAP_FAILED) {
		handle_error("Error at mmap");
	}
	fprintf(stderr, "Notification: A file mapping in memory was created, for the destination file\n");
	if (-1 == close(fd)) {
		handle_error("Error at close");
	}
	strcpy(map_fd_addr, buffer);
	if (-1 == msync(map_fd_addr, length, MS_SYNC)) {
		handle_error("Error at msync");
	}
	if (-1 == munmap(map_fd_addr, length)) {
		handle_error("Error at munmap");
	}
}
int ask_suprascrie(char *dest) {
	char raspuns = '$';
	printf("Doriti sa suprascrieti fisierul destinatie %s? (Y/N) \n", dest);
	do {
		scanf("%c", &raspuns);
	} while(raspuns != 'Y' && raspuns != 'N');
	int flag_suprascrie;
	if(raspuns == 'Y') {
		flag_suprascrie = 1;
	} else {
		flag_suprascrie = 0;
	}
	return flag_suprascrie;
}
