/*
   Iată o soluție incompletă, în sensul că s-a omis codul efectiv de punere și de scoatere a lacătului pentru acces exclusiv la fișierul de date partajat,
   precum și tratarea anumitor excepții (i.e., atunci când se încearcă extragerea unei cantități mai mari decât cea disponibilă în rezervorul stației PECO).
*/
// TODO: respectivele omisiuni vă rămân ca temă să le completați dvs. !!!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#define handle_error(msg) \
    { perror(msg); exit(EXIT_FAILURE); }
void initializare_fisier_date();
/*
*  Pentru început, avem nevoie de o rutină ajutătoare, care să creeze conținutul inițial al fișierului de date partajat.
*  Se va crea fișierul și va conține un număr real, stocat în format binar, reprezentând cantitatea inițială de combustibil din rezervor.
*  Vom apela programul cu opțiunea -i pentru a invoca această rutină de inițializare.
*/

void afisare_fisier_date();
/*
*  Similar, la finalul tuturor actualizărilor, vom avea nevoie de o altă rutină ajutătoare, care să ne afișeze conținutul final din rezervor.
*  Vom apela programul cu opțiunea -o pentru a invoca această rutină de afișare finală.
*/

void secventa_de_actualizari(int n, char* nr[]);

/*
*  Rutina principală a programului, care va executa secvența de actualizări primită în linia de comandă, în manieră concurentă,
*  conform celor specificate în enunțul problemei.
*  Observație: rutina principală nu este implementată în întregime mai jos, ci vă rămâne dvs. sarcina să completați "bucățile" de cod
*  care lipsesc (indicate prin comentarii TODO).
*/

int main(int argc, char* argv[])
{
	if(argc == 1)
	{
		fprintf(stderr, "Eroare: programul trebuie apelat cu optiunile: -i, -o, sau cu o secventa de numere reale...\n");  return 1;
	}

	/* Cazul de inițializare a fișierului de date. */
	if( strcmp(argv[1], "-i") == 0 )
	{
		initializare_fisier_date();  return 0;
	}

	/* Cazul de afișare a fișierului de date. */
	if( strcmp(argv[1],"-o") == 0 )
	{
		afisare_fisier_date();  return 0;
	}

	/* Cazul general: actualizarea fișierului de date. */
	secventa_de_actualizari(argc, argv);
	return 0;
}


void initializare_fisier_date()
{/* Funcționalitate: inițializarea fișierului de date. */

	int fd;
	float val;

	printf("Dati cantitatea initiala de combustibil (i.e., un numar real pozitiv): ");
	while( (1 != scanf("%f", &val)) && (val < 0) ) { fprintf(stderr,"Eroare: nu ati introdus un numar real pozitiv! Incercati din nou..."); }

	if(-1 == (fd = open("peco.bin", O_CREAT|O_RDWR, 0600) ) ) {
		handle_error("Eroare la crearea fisierului de date...");  
		exit(2);
	}
	
	// TO DO - change this write to file mapping 
	ftruncate(fd, sizeof(float));
	float *map_addr = mmap(NULL, // address of mapping
						sizeof(float), // size of the source file will be the length
						PROT_READ|PROT_WRITE, // Doar pentru citire
						MAP_SHARED, // PROT_READ merge cu MAP_PRIVATE
						fd, // fisierul mapat
						0);
	if (map_addr == MAP_FAILED) {
		handle_error("Error at mmap");
	}
	fprintf(stderr, "Notification: A file mapping in memory was created, for the source file\n");
	if (-1 == close(fd)) {
		handle_error("Error at close");
	}
	*map_addr = val;
	if (-1 == msync(map_addr, sizeof(float), MS_SYNC)) {
		handle_error("Error at msync");
	}
	if (-1 == munmap(map_addr, sizeof(float))) {
		handle_error("Error at munmap");
	}
}


void afisare_fisier_date()
{/* Funcționalitate: afișarea fișierului de date. */

	int fd;
	float val;

	if ( -1 == (fd = open("peco.bin", O_RDONLY))) {
		handle_error("Eroare la deschiderea pentru afisare a fisierului de date...");  
		exit(4); 
	}
	
	// TO DO - change this read to file mapping
	// stiu ca sunt sizeof(float) octeti MEREU in acel fisier
	
	float *map_addr = mmap(NULL, // address of mapping
						sizeof(float), // size of the source file will be the length
						PROT_READ, // Doar pentru citire
						MAP_PRIVATE, // PROT_READ merge cu MAP_PRIVATE
						fd, // fisierul mapat
						0);
	if (-1 == close(fd)) {
		handle_error("Error at close");
	}
	val = *map_addr;
	if (-1 == munmap(map_addr, sizeof(float))) {
		handle_error("Error at munmap");
	}
	printf("Stocul final de combustibil este: %f litri de combustibil.\n", val);
}


void secventa_de_actualizari(int n, char* nr[])
{ /* Funcționalitate: realizarea secvenței de operații de actualizare a fișierului de date. */

	float valoare, stoc;
	int fd, i;

	if (-1 == (fd = open("peco.bin", O_RDWR) ) )
	{
		handle_error("Eroare la deschiderea pentru actualizare a fisierului de date...");  
		exit(6); 
	}
	float *map_addr = mmap(NULL, // address of mapping
						sizeof(float), // size of the source file will be the length
						PROT_READ|PROT_WRITE, // Doar pentru citire
						MAP_SHARED, // PROT_READ merge cu MAP_PRIVATE
						fd, // fisierul mapat
						0);
	if (map_addr == MAP_FAILED) {
		handle_error("Error at mmap");
	}
	fprintf(stderr, "[PID: %d] Notification: A file mapping in memory was created, for the source file\n", getpid());
	
	srandom( getpid() ); //  Inițializarea generatorului de numere aleatoare.

	for(i = 1; i < n; i++) // Iterăm prin secvența de numere reale specificată în linia de comandă.
	{
		if(1 != sscanf(nr[i], "%f", &valoare))  // Facem conversie de la string la float.
		{
			fprintf(stderr, "[PID: %d] Eroare: ati specificat o valoare invalida drept argument de tip numar real : %s !\n", getpid(), nr[i] );
			exit(7);
		}
		
		struct flock lacat;
		lacat.l_type = F_WRLCK;
		lacat.l_whence = SEEK_SET;
		lacat.l_start = 0;
		lacat.l_len = sizeof(float);
		
		if(-1 == fcntl(fd, F_SETLKW, &lacat)) { 
			if(errno == EINTR) {
				fprintf(stderr,"[ProcesID:%d] Eroare, apelul fcntl a fost intrerupt de un semnal...", getpid());
			}
			else {
				fprintf(stderr,"[ProcesID:%d] Eroare la blocaj...", getpid());
			}
			perror("\tMotivul erorii");
			exit(4);
		}
		else {
			printf("[ProcesID:%d] Blocaj reusit!\n", getpid());
		}
		
		if(-1 == lseek(fd, 0, SEEK_SET) ) {
			perror("Eroare la repozitionarea in fisierul de date, pentru citire...");  
			exit(8); 
		}
		// in loc de citire iau ce e deja la acea adresa
		stoc = *map_addr;
		//if (-1 == read(fd, &stoc, sizeof(float))) {
			//perror("Eroare la citirea valorii din fisierul de date...");  
			//exit(9); 
		//}

		printf("[PID: %d] Se adauga/extrage in/din rezervor cantitatea de %f litri de combustibil.\n", getpid(), valoare );
		if(stoc + valoare < 0) {
			char raspuns = '?';
			printf("Cantitate ramasa in stoc : %f litri --- Cantitatea dorita spre extragere : %f litri\n", stoc, -valoare);
			printf("Afisati pe ecran ce doriti sa faceti mai departe: \n");
			while(raspuns != '1' && raspuns != '2') {
				printf("Varianta 1 : Se refuza complet operatia de extragere\n");
				printf("Varianta 2 : Se extrage toata cantitatea care a ramas in stoc\n");
				printf("Ce alegeti?: ");
				scanf("%c", &raspuns);
			}
			if(raspuns == '1') {
				printf("Operatia curenta de extragere nu s-a efectuat deloc!\n");
			} else {
				printf("Operatia curenta de extragere s-a efectuat doar partial!\n");
				stoc = 0;
			}	
		} else {
			stoc += valoare;
		}
		
		if(-1 == lseek(fd, 0, SEEK_SET) ) // Repoziționăm cursorul la începutul fișierului.
		{
			handle_error("Eroa8re la repozitionarea in fisierul de date, pentru scriere...");  
		}
		*map_addr = stoc;
		lacat.l_type = F_UNLCK;
		if(-1 == fcntl(fd, F_SETLKW, &lacat)) {
			perror("Eroare la deblocaj"); 
			exit(9);
		}
		lacat.l_type = F_WRLCK;
		sleep( random() % 5 ); // Facem o pauză aleatoare, de maxim 4 secunde, înainte de a continua cu următoarea iterație.
	}     
	if (-1 == close(fd)) {
		handle_error("Error at close");
	}
	fprintf(stderr, "[PID: %d] Stergem maparea\n", getpid());
	if (-1 == msync(map_addr, sizeof(float), MS_SYNC)) {
		handle_error("Error at msync");
	}
	if (-1 == munmap(map_addr, sizeof(float))) {
		handle_error("Error at munmap");
	}
	printf("[PID: %d] Am terminat de procesat secventa de actualizari.", getpid());
}
