#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#define BLOCK_SIZE 4096
#define handle_error(msg) \
    { perror(msg); exit(EXIT_FAILURE); }
int fs, f_SET = 0, d_SET = 0, b_SET = 0, sourceNr = 0;
char *sourceName[BLOCK_SIZE], d_separator;
int f_list[BLOCK_SIZE * 2], b_list[BLOCK_SIZE * 2], f_Args = 0, b_Args = 0;

int is_dig(char ch);
int parsare(char *text, int *list, int *count);
int printLine(char *line); 
void doCut(char *sourceName);

int main(int argc, char *argv[]) {
	
	int i;
	int BAD_FLAG = 0;
	for(i = 1; i < argc; ++i) {
		if(!strcmp(argv[i], "-f") && !f_SET) {
			if(i + 1 == argc) BAD_FLAG = 1;
			else BAD_FLAG = parsare(argv[i + 1], f_list, &f_Args);
			if(BAD_FLAG) {
				break;
			}
			f_SET = 1;
			i++;
			continue;
		}
		if(!strcmp(argv[i], "-b") && !b_SET) {
			if(i + 1 == argc) BAD_FLAG = 1;
			else BAD_FLAG = parsare(argv[i + 1], b_list, &b_Args);
			if(BAD_FLAG) {
				break;
			}
			b_SET = 1;
			i++;
			continue;
		}
		if(!strcmp(argv[i], "-d") && !d_SET) {
			if(i + 1 == argc) BAD_FLAG = 1;
			else {
				if(strlen(argv[i + 1]) != 1) {
					BAD_FLAG = 1;
				}
				d_separator = argv[i + 1][0];
			}
			if(BAD_FLAG) {
				break;
			}
			d_SET = 1;
			i++;
			continue;
		}
		sourceName[sourceNr++] = argv[i];
	}
	if(BAD_FLAG == 1) {
		fprintf(stderr, "Eroare! Nu ati respectat formatul impus de comanda cut!");
		exit(1);
	}
	if(sourceNr == 0) {
		fprintf(stderr, "Eroare! Nu ati specificat nici un fisier sursa");
		exit(2);
	}
	if(f_SET && b_SET) {
		fprintf(stderr, "Un singur tip de lista poate fi specificat!");
		exit(3);
	}
	if(d_SET && !f_SET) {
		fprintf(stderr, "Un delimitator poate fi declarat doar daca avem si fielduri (-f)");
		exit(4);
	}
	if(f_SET && !d_SET) {
		fprintf(stderr, "Este nevoie de un delimitator!");
		exit(5);
	}
	for(int i = 0; i < sourceNr; ++i) {
		doCut(sourceName[i]);
	}
}
void doCut(char *sourceName) {
	int fs = open(sourceName, O_RDONLY);
	if(fs == -1) {
		perror("Eroare la deschiderea fisierelor sursa");
		exit(6);
	}
	long PageSize = sysconf(_SC_PAGE_SIZE);
	if (PageSize == -1) {
		handle_error("Error at sysconf");
	}
	fprintf(stderr, "Notification: the page size on your hardware system is: %ld bytes.\n", PageSize);
	struct stat sb;
	if (fstat(fs, &sb) == -1) {  /* To obtain file size */
		handle_error("Error at fstat");
	}
	char *map_addr = mmap (NULL, // address of mapping
						sb.st_size, // size of the source file will be the length
						PROT_READ, // Doar pentru citire
						MAP_PRIVATE, // PROT_READ merge cu MAP_PRIVATE
						fs, // fisierul mapat
						0); // incepem sa mapam de la inceputul fisierului (este multiplu de dim paginii)
	if (map_addr == MAP_FAILED) {
		handle_error("Error at mmap");
	}
	fprintf(stderr, "Notification: A file mapping in memory was created, for the source file\n");
	if (-1 == close(fs)) {
		handle_error("Error at close");
	}
	
	char currentLine[BLOCK_SIZE * 4];
	int cursor = 0;
	for(int i = 0; i < sb.st_size; ++i) {
		if(*(map_addr + i) != '\n') {
			currentLine[cursor++] = *(map_addr + i);
		} else {
			// avem newline
			currentLine[cursor] = '\0';
			printLine(currentLine);
			cursor = 0;
		}
	}
	if (-1 == munmap(map_addr, sb.st_size)) {
		handle_error("Error at munmap");
	}
}

int is_dig(char ch) {
	return (ch >= '0' && ch <= '9');
}
int parsare(char *text, int *list, int *count) {
	// lista trebuie sa respecte formatul : 1,2,4,13,15..
	int currentNumber = 0;
	for(int i = 0; i < strlen(text); ++i) {
		if(is_dig(text[i])) {
			currentNumber = currentNumber * 10 + text[i] - '0';
		} else {
			if(!currentNumber || text[i] != ',') {
				return 1;
			}
			*(list + *count) = currentNumber;
			*count += 1;
			currentNumber = 0;
		}
	}
	if(currentNumber == 0) {
		return 1;
	}
	*(list + *count) = currentNumber;
	*count += 1;
	return 0;
}
int printLine(char *line) {
	int len = strlen(line);
	if(b_SET) {
		for(int i = 0; i < b_Args; ++i) {
			int byte = b_list[i] - 1;
			if(byte < len) {
				printf("%c", line[byte]);
			}
		}
		printf("\n");
	}
	if(d_SET) {
		// I KNOW THAT
		// F IS ALSO SET
		// B IS NOT SET  
		int wordCount = 0, need[BLOCK_SIZE * 2];
		memset(need, 0, sizeof(need));
		for(int i = 0; i < f_Args; ++i) {
			need[f_list[i]] = 1;
		}
		char word[BLOCK_SIZE * 2];
		int cursor = 0;
		for(int i = 0; i < len; ++i) {
			if(line[i] == d_separator) {
				wordCount++;
				word[cursor] = '\0';
				if(need[wordCount]) {
					printf("%s ", word);
				}
				cursor = 0;
			} else {
				word[cursor++] = line[i];
			}
		}
		if(cursor > 0) {
			word[cursor] = '\0';
			wordCount++;
			if(need[wordCount]) {
				printf("%s ", word);
			}
			cursor = 0;
		}
		printf("\n");
	}
	return 0;
}
