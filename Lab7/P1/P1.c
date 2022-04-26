#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

void cauta_depozit(int cod, float cantitate);
int depozit_descr;
FILE *instructions_descr;

int main(int argc, char **argv) {
	
	if(argc < 3) {
		fprintf(stderr, "Eroare! Programul trebuie apelat cu un fisier binar pentru depozit si un fisier binar pentru instructiuni!");
	}
	if(-1 == (depozit_descr = open(argv[1], O_RDWR, O_TRUNC))) {
		perror("Eroare la deschiderea fisierului depozit: ");
		exit(1);
	}
	if(NULL == (instructions_descr = fopen(argv[2], "r"))) {
		perror("Eroare la deschiderea fisierului de instructiuni: ");
		exit(1);
	}
	
	struct flock lacat_blocaj;
	lacat_blocaj.l_whence = SEEK_SET;
	lacat_blocaj.l_start = 0;
	lacat_blocaj.l_type = F_WRLCK;
	lacat_blocaj.l_len = 1;
	
	struct flock lacat_deblocaj;
	lacat_deblocaj.l_whence = SEEK_SET;
	lacat_deblocaj.l_start = 0;
	lacat_deblocaj.l_type = F_UNLCK;
	lacat_deblocaj.l_len = 1;
		
	while(1) {
		
		int cod_produs, cod_r;
		cod_r = fscanf(instructions_descr, "%d", &cod_produs);
		if(cod_r == EOF) {
			break;
		}
		if(cod_r != 1) {
			perror("Eroare la citirea dcin fisierul de instructiuni: ");
			exit(2);
		}
		float cantitate;
		cod_r = fscanf(instructions_descr, "%f", &cantitate);
		if(cod_r == EOF) {
			break;
		}
		if(cod_r != 1) {
			perror("Eroare la citirea din fisierul de instructiuni: ");
			exit(2);
		}
		
		printf("[ProcesID:%d] Se proceseaza operatia de adaugare/extragere de %f din produsul cu codul %d\n", getpid(), cantitate, cod_produs);
		
		if(-1 == fcntl(depozit_descr, F_SETLKW, &lacat_blocaj)) {
			if(errno == EINTR) {
				fprintf(stderr,"[ProcesID:%d] Eroare, apelul fcntl a fost intrerupt de un semnal...", getpid());
			}
			else {
				fprintf(stderr,"[ProcesID:%d] Eroare la blocaj...", getpid());
			}
			perror("\tMotivul erorii");
			exit(4);
		} else {
			printf("[ProcesID:%d] Blocaj reusit!\n", getpid());
		}
		
		cauta_depozit(cod_produs, cantitate);
		
		if(-1 == fcntl(depozit_descr, F_SETLKW, &lacat_deblocaj)) {
			if(errno == EINTR) {
				fprintf(stderr,"[ProcesID:%d] Eroare, apelul fcntl a fost intrerupt de un semnal...", getpid());
			}
			else {
				fprintf(stderr,"[ProcesID:%d] Eroare la deblocaj...", getpid());
			}
			perror("\tMotivul erorii");
			exit(4);
		} else {
			printf("[ProcesID:%d] Deblocaj reusit!\n", getpid());
		}
	}
	fclose(instructions_descr);
	if(-1 == close(depozit_descr)) {
		perror("Eroare la inchiderea fisierului depozit! ");
		exit(6);
	}
}
void cauta_depozit(int cod, float cantitate) {
	
	if(-1 == lseek(depozit_descr, 0, SEEK_SET)) {
		perror("Eroare la repozitionare! ");
		exit(3);	
	}
	
	int gasit = 0;
	
	while(1) {
		int depozit_cod_produs, depozit_cod_r;
		depozit_cod_r = read(depozit_descr, &depozit_cod_produs, sizeof(int));
		if(depozit_cod_r == 0) {
			break;
		}
		if(depozit_cod_r == -1) {
			perror("Eroare la citirea din fisierul depozit! ");
			exit(2);
		}
		float depozit_cantitate;
		depozit_cod_r = read(depozit_descr, &depozit_cantitate, sizeof(float));
		if(depozit_cod_r == 0) {
			break;
		}
		if(depozit_cod_r == -1) {
			perror("Eroare la citirea din fisierul depozit! ");
			exit(2);
		}
		if(depozit_cod_produs == cod) {
			gasit = 1;
			printf("[ProcesID:%d] Am gasit produsul cu codul %d, ii facem actualizare!\n", getpid(), cod);
			
			if(-1 == lseek(depozit_descr, -sizeof(float), SEEK_CUR)) {
				perror("Eroare la repozitionare! ");
				exit(3);
			}
			// AICI AR TREBUI SA PUN UN LACAT
			
			// Aici modificam
			if(depozit_cantitate + cantitate < 0) {
				fprintf(stderr, "Eroare! Nu putem extrage cantitatea %f din cantitatea %f\n", cantitate, depozit_cantitate);
				exit(4);
			} else {
				depozit_cantitate += cantitate;
				if(-1 == write(depozit_descr, &depozit_cantitate, sizeof(float))) {
					perror("Eroare la scrierea in fisierul depozit: ");
					exit(5);
				}
			}
			break;
		}
	}
	
	if(gasit == 0) {
		printf("[ProcesID:%d] Nu am gasit produsul cu codul %d, incercam sa-l adaugam!\n", getpid(), cod);
		
		if(-1 == lseek(depozit_descr, 0, SEEK_END)) {
			perror("Eroare la repozitionare! ");
			exit(3);	
		}
		
		if(cantitate < 0.0) {
			fprintf(stderr, "Eroare! S-a incercat o operatie de extragere din produsul cu codul %d (care nu exista in depozit) \n", cod);
			exit(7);
		} else {
			if(-1 == write(depozit_descr, &cod, sizeof(int))) {
				perror("Eroare la scrierea in fisierul depozit: ");
				exit(5);
			} 
			if(-1 == write(depozit_descr, &cantitate, sizeof(float))) {
				perror("Eroare la scrierea in fisierul depozit: ");
				exit(5);
			} 
		}
	}
}
