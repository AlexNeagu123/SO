/*
  Programul: supervisor.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#define MAX_NO_WORKERS      10
#define MAX_LENGTH_FILENAME 50

int main()
{
	int nr = 0, k, NW = 0, i, pid, sum = 0;
	
	char  input_filenames[MAX_NO_WORKERS][MAX_LENGTH_FILENAME];
	char output_filenames[MAX_NO_WORKERS][MAX_LENGTH_FILENAME];
	FILE *fdi[MAX_NO_WORKERS], *fdo;
 
	printf("Dati numarul de procese workers: ");
	while(1 != scanf("%d", &NW) ) { fprintf(stderr, "Trebuie sa introduceti un numar intreg pozitiv! Incercati din nou...\n"); }
	if( (NW < 1) || (NW > MAX_NO_WORKERS) )
	{
		fprintf(stderr,"Date de intrare incorecte.\n");  
		exit(1);
	}

	for (i = 1; i <= NW; ++i)
	{
		sprintf( input_filenames[i], "f%di.txt", i);
		sprintf(output_filenames[i], "f%do.txt", i);
	}


	// Pasul #1: Citirea numerelor de la tastatură și crearea celor NW fișiere de intrare.

	for (i = 1; i <= NW; ++i)
	{
		if( NULL == (fdi[i-1] = fopen(input_filenames[i], "w")) )
		{
			fprintf(stderr,"[Supervisor] Eroare la deschiderea fisierului de intrare %s.\n", input_filenames[i]);
			perror("Cauza erorii: ");
			exit(2);
		}
	}

	printf("Introduceti secventa de numere (0 pentru terminare) :\n");
	k = 0;
	do{
		while(1 != scanf("%d", &nr) ) { fprintf(stderr, "Trebuie sa introduceti un numar intreg nenul! Incercati din nou...\n"); }
		if(nr != 0)
			fprintf(fdi[k], "%d ",nr);
		k = (k + 1) % NW;
	} while(nr != 0);
 
	for (i = 1; i <= NW; ++i)
	{
		fclose(fdi[i-1]);
	}
    

	// Pasul #2: Crearea celor NW procese fii de tip worker, i.e. care vor executa programul din sursa worker.c, cu parametri diferiți.

	for (i = 1; i <= NW; ++i)
	{
		if(-1 == (pid = fork()) )
		{
			fprintf(stderr,"[Supervisor] Eroare la apelul fork() pentru procesul worker al %d-lea.\n", i);
			perror("Cauza erorii: ");
			exit(3);
		}
		
		if (pid == 0)
		{
			// În procesul fiu apelez cu execl programul worker
			execl("worker", "Programul worker", input_filenames[i], output_filenames[i], NULL);
			fprintf(stderr,"[Supervisor] Eroare la apelul execl() pentru procesul worker al %d-lea.\n", i);
			perror("Cauza erorii: ");
			exit(4);
		}
		///else
		///    wait (NULL);   Notă: ar fi greșit să așteptăm aici terminarea fiului, căci ar însemna că workerii se vor executa secvențial, nu în paralel!
	}

	// IMPORTANT: Abia după ce supervisorul a startat toate procesele worker, le va aștepta să se termine de executat, pe rând.
	// (!! Revedeți "Observația #2" formulată în rezolvarea exercițiului [N childs] din laboratorul precedent !!)

	for (i = 1; i <= NW; ++i)
	{
		wait(NULL);
	}


	// Pasul #3: Citirea celor NW sume parțiale din fișierele de ieșire și adunarea lor.


	for (i = 1; i <= NW; ++i)
	{
		if (NULL == (fdo = fopen(output_filenames[i], "r")) )
		{
			fprintf(stderr,"[Supervisor] Eroare la deschiderea fisierului de iesire %s.\n", output_filenames[i]);
			perror("Cauza erorii: ");
			exit(5);
		}
		
		if (1 != fscanf(fdo, "%d", &nr) )
		{
			fprintf(stderr,"[Supervisor] Eroare la citirea celei de a %d-a sume partiale.\n", i);
			exit(6);
		}
		else
			sum += nr;
		
		fclose(fdo);
	}
	
	/* Se mai poate optimiza: combinăm cele două for-uri de mai sus într-unul singur, pentru a "paraleliza" citirea unei sumei din fișier cu așteptarea terminării unui alt worker.

	   Pentru a realiza aceasta, se pot salva PID-urile fiilor într-un tabel, la momentul creării lor, pentru ca la final, când returnează un apel wait(NULL),
	   să știu din ce fișier trebuie să citesc suma parțială corespunzătoare acelui fiu care tocmai s-a terminat!

	   O altă idee, în loc de cea pe baza PID-urilor fiilor de mai sus, ar putea fi următoarea: dacă limităm numărul de workeri NW la maxim 255,
	   putem rescrie programul worker să primească și numărul lui de ordine i, cuprins între 1 și NW, iar la final să returneze exact valoarea i,
	   în caz de succes, respectiv 0, în cazul celor două ramuri cu erori din program. Apoi rescriem supervisorul să aștepte cu wait(&status), pentru a afla
	   numărul de ordine i al acelui fiu care se termină, si astfel să știm din ce fișier să citim suma parțială corespunzătoare acelui fiu care tocmai s-a terminat!
	   
	   //TODO: încercați singuri să rescrieți cele două bucle for într-una singură, conform celor explicate mai sus.
	*/


	// Pasul #4: La final, afișarea sumei totale.
	printf("[Supervisor] Suma calculata a secventei introduse este: %d.\n", sum);

	// În plus, opțional, putem face "curățenie": ștergem fișierele de intrare și de iesire (pentru a pregăti o nouă execuție...)
	for (i = 1; i <= NW; ++i)
	{
		remove( input_filenames[i]);
		remove(output_filenames[i]);
	}
	return 0;
}
