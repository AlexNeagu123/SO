/*
*  Program: sig_ex3.c
*
*  Funcționalitate: realizează o buclă finită de 10 secunde, cu tratarea ("coruperea") semnalelor de întrerupere și terminare a execuției programului.
*/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

/* Definim handler-ul propriu de semnal pentru SIGINT și SIGQUIT */
void my_handler(int nr_sem)
{
  /* Acțiunea de tratare configurată de programator... */
  printf("Nu mai tasta CTRL+%s caci nu are efect.\n", (nr_sem==SIGINT ? "C":"\\") );
}

int main()
{
  int i;

  /* Tratarea ("coruperea") semnalelor SIGINT și SIGQUIT */
  signal( SIGINT, my_handler);
  signal(SIGQUIT, my_handler);
  
  /* Porțiune de cod pe care ^C și ^\ sunt corupte */
  printf("Inceput portiune de cod pe care CTRL+C si CTRL+\\ sunt corupte...\n");
  for(i=0;i<10;i++) { printf("Portiune corupta...\n"); sleep(1); }
  printf("Sfarsit portiune corupta.\n");
  
  /* Refacerea comportamentului implicit pentru cele două semnale */
  signal(SIGINT, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);

  /* Porțiune de cod pe care ^C si ^\ nu sunt corupte */
  for(i=0;i<10;i++) { printf("Portiune necorupta...\n"); sleep(1); }
  printf("Sfarsit program.\n");

  return 0;
}
