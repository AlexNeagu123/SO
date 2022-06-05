/*
  Program: demo_data-race2_prod+cons_v2_synced.c (Varianta sincronizată !  ==> nu mai suferă de data-race-uri !!!)

  Funcționalitate: se ilustrează cooperarea dintre două procese, tată și fiu, folosind memorie comună.

  Tatăl implementează un producător, ce va "produce" 20 de texte, fiecare de lungime 10, la intervale de tip aleatoare, pe care le va "comunica"
  partenerului său, consumatorul, prin scriere într-o mapare anonimă în memorie, într-un același buffer, refolosit pentru fiecare scriere.

  Iar fiul implementează un consumator, ce va "consuma" 20 de texte, fiecare de lungime 10, la intervale de tip aleatoare, pe care le va "primi"
  de la partenerul său, producătorul, prin citirea lor dintr-o mapare anonimă în memorie, dintr-un același buffer în care le-a scris producătorul.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define handle_error(msg) \
    { perror(msg); exit(EXIT_FAILURE); }

#define NR_TEXTE  20
#define DIM_TEXTE 10
#define GUARD_SPACE 5

void producer_work(); // Funcție definită mai jos, ce conține implementarea funcționalității de producător a tatălui

void consumer_work(); // Funcție definită mai jos, ce conține implementarea funcționalității de consumator a fiului


char *map_addr;
char *pSharedBuffer;
char *pSharedToken;

// definim o funcție macrou, cu trei implementări alternative, ce va fi inclusă în codurile ambelor procese
#define WAIT_FOR_TOKEN(digit)  \
  /* varianta 1 de sincronizare: așteptare ocupată */  \
	/*  do{ } while( *pSharedToken != digit);  */  \
	/* I am "spinning" on the CPU, testing repeatedly for zero... */  \
\
  /* varianta 2 de sincronizare: așteptare blocantă */  \
	/*  do{ sleep(1); } while( *pSharedToken != digit);  */  \
	/* I will sleep for 1 second in the waiting state, then I will become ready, then running and test again for zero... */  \
\
  /* varianta 3 de sincronizare: the minimal non-spinning waiting */  \
	do{ sleep(0); } while( *pSharedToken != digit);  \
	/* I am going to the ready state (i.e., I preempt myself, but without an effective sleep), then running and test again for zero... */


int main()
{
    pid_t pid_fiu;
    size_t length;

    length = DIM_TEXTE + GUARD_SPACE;   // Sau, pentru acele valori, putem și astfel:  length = 100;

    /* Determinăm dimensiunea paginilor corespunzătoare arhitecturii hardware a calculatorului dvs. */
    long PageSize = sysconf(_SC_PAGE_SIZE);
    if (PageSize == -1)
        handle_error("Error at sysconf");
    fprintf(stderr, "Notification: The page size on your hardware system is: %ld bytes.\n", PageSize);

    /* Calculăm partea întreagă superioară a raportului dintre valorile length și PageSize. */
    long mmap_size_in_pages = ( length % PageSize ? 1 : 0 ) + ( length / PageSize );


    /* Step 1: Crearea unei mapări anonime, partajate și cu drepturi de citire+scriere. Neapărat înaintea apelului de clonare a procesului... */

    map_addr = mmap( NULL,                       // Se va crea o mapare începând de la o adresă page-aligned aleasă de kernel (și returnată în map_addr)
                     length,                     // Lungimea dorită, preferabil multiplu de dimensiunea paginii (oricum, se vor aloca un număr de pagini "complete")
                     PROT_READ | PROT_WRITE,     // Tipul de protecție a mapării: paginile mapării vor permite accese în citire și scriere
                     MAP_SHARED | MAP_ANONYMOUS, // Maparea este anonimă și partajată, pentru a avea memorie comună între procesele tată și fiu
                     -1,                         // La descriptorul de fișier se pune -1, conform documentației
                     0                           // Offset-ul (deplasamentul) nu este luat în seamă, conform documentației
                   );
    if (map_addr == MAP_FAILED)
        handle_error("Error at mmap");
    fprintf(stderr, "Notification: An anonymous shared mapping in memory was created, with read/write permission, and with the length: %ld.\n", length);
    fprintf(stderr, "Notification:   Actually, the length/size of the mapping in memory is: %ld page(s).\n", mmap_size_in_pages); 

    pSharedBuffer = map_addr + GUARD_SPACE;  // de la această adresă va începe bufferul de capacitate = 1, folosit pentru transmiterea textelor
    /* Observație: am rezervat la începutul mapării anonime un spațiu nefolosit, de dimensiune GUARD_SPACE octeți,
       pe care-l vom folosi ulterior pentru implementarea unui mecanism de sincronizare între cele două procese. */

    pSharedToken = map_addr + 0;  // la această adresă va fi octetul cu rol de token, folosit pentru implementarea sincronizării necesare
    /* Observație: vom folosi drept token chiar octetul de la prima adresă din spațiul nefolosit rezervat la începutul mapării anonime,
       spațiu de dimensiune GUARD_SPACE octeți, după care urmează spațiul folosit pentru bufferul de comunicare a textelor între cele două procese.
	   Valoarea inițială a tokenului este zero (datorită modului de inițializare a mapării anonime). */


    /* Step 2: Crearea unui proces fiu. */
    if(-1 == (pid_fiu = fork()) )
    {
        perror("Eroare la fork");  
        return 1;
    }

    /* Step 3: Ramificarea execuției în cele două procese, tată și fiu. */
    if(pid_fiu == 0)
    {   /* Zona de cod executată doar de către fiu. */

        fprintf(stderr, "[Consumer] Notification: sunt procesul fiu, cu PID-ul: %d.\n", getpid());
        consumer_work();
    }
    else
    {   /* Zona de cod executată doar de către părinte. */

        fprintf(stderr, "[Producer] Notification: sunt procesul tata, cu PID-ul: %d.\n", getpid());
        producer_work();

        wait(NULL); /* acest apel are doar un rol "estetic": să împiedice afișarea prompterului shell-ului înainte de a se termina și procesul fiu
                       (fiul fiind rulat în background, nu este așteptat sa se termine și el de către shell, înainte de a reafișa prompterul !)  */ 
    }

    /* Zona de cod comună, executată de către ambele procese */

    // La final, "ștergem/distrugem" maparea anonimă creată anterior. Oricum, aceasta se făcea și implicit, la finalul programului.
    if (-1 == munmap(map_addr, length) )
        handle_error("Error at munmap");

    fprintf(stderr, "Notification: sfarsitul executiei procesului %s.\n\n", pid_fiu == 0 ? "fiu" : "parinte" );
    return EXIT_SUCCESS;
}

void producer_work()
{
    int i;
    char texte_de_scris[NR_TEXTE][DIM_TEXTE];

    srandom( getpid() ); // Inițializarea generatorului de numere aleatoare.

    // Inițializarea textelor.
    for(i = 0 ; i < NR_TEXTE ; i++)
    {
        snprintf(texte_de_scris[i],DIM_TEXTE,"Textul %2d",i); // Atenție, e important: pe poziția a 10-a din text se va stoca zero-ul ce indică null-terminated string!
    }

    // Scriem textele în bufferul pentru mesaje, stocat în maparea anonimă din memorie.
    for(i = 0; i < NR_TEXTE; i++)
    {
        strcpy(pSharedBuffer, texte_de_scris[i]);  // Scriem al i-lea text în bufferul partajat de cele două procese
        fprintf(stderr, "[Producer] Notification: I wrote the %dth text, \"%s\", in the mapping.\n", i, texte_de_scris[i]);

        *pSharedToken = 1;  // Suprascriem 0 cu 1 în token, i.e. acum urmează rândul consumatorului să citească un text.

        sleep( random() % 3 );  // Facem o pauză aleatoare, de maxim 2 secunde, înainte de a continua cu următoarea iterație.

        WAIT_FOR_TOKEN(0)  // Așteptăm ca tokenul să redevină 0, i.e. producătorul așteaptă să fie din nou rândul său să scrie un text. 
    }

    fprintf(stderr, "[Producer] Notification: I finished my job!\n");
    return; //exit(EXIT_SUCCESS);
}

void consumer_work()
{
    int i;
    char texte_de_citit[NR_TEXTE][DIM_TEXTE];

    srandom( getpid() ); // Inițializarea generatorului de numere aleatoare.

    // Citim textele din bufferul pentru mesaje, stocat în maparea anonimă din memorie.
    for(i = 0; i < NR_TEXTE; i++)
    {
        WAIT_FOR_TOKEN(1)  // Așteptăm ca tokenul să (re)devină 1, i.e. consumatorul așteaptă să fie (din nou) rândul său să citească un text. 

        strcpy(texte_de_citit[i], pSharedBuffer);  // Citim al i-lea text din bufferul partajat de cele două procese
        fprintf(stderr, "[Consumer] Notification: I read the %dth text, \"%s\", from the mapping.\n", i, texte_de_citit[i]);

        *pSharedToken = 0;  // Suprascriem 1 cu 0 în token, i.e. acum urmează rândul producătorului să scrie un text.

        sleep( random() % 3 ); // Facem o pauză aleatoare, de maxim 2 secunde, înainte de a continua cu următoarea iterație.
    }

    fprintf(stderr, "[Consumer] Notification: I finished my job!\n");
    return; //exit(EXIT_SUCCESS);
}
