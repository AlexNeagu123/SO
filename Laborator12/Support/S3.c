/*
*  Program: sig_ex4.c
*
*  Funcționalitate: ilustrează modul de blocare a primirii unor semnale.
*/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int main()
{
  sigset_t  base_mask, waiting_mask;
  int i;

  printf("Inceputul executiei programului.\n%s%s\n%s\n\n",
         "Puteti oricand tasta CTRL+C sau CTRL+Z pentru a termina/stopa programul, ",
         "dar acest lucru se va intampla efectiv doar dupa primele 10 secunde de la start.",
         "Iar daca nu tastati nici CTRL+C si nici CTRL+Z, programul se va termina singur dupa 20 de secunde.");

  sigemptyset(&base_mask);
  sigaddset(&base_mask, SIGINT);
  sigaddset(&base_mask, SIGTSTP);

  /* Se blochează semnalele CTRL+C și CTRL+Z, ce pot fi generate de la tastatură de către utilizator. */
  sigprocmask(SIG_SETMASK, &base_mask, NULL);

  /* Un interval de așteptare de 10 secunde. */
  for(i=0; i<10; i++)
  { sleep(1);  printf("Time: %d seconds\n", i+1); }

  /* Se verifică dacă există semnale SIGINT sau SIGTSTP în așteptare. */
  sigpending(&waiting_mask);

  if( sigismember(&waiting_mask, SIGINT) )
  { printf("Userul a incercat sa termine procesul in primele 10 secunde.\n"); }

  if( sigismember(&waiting_mask, SIGTSTP) )
  { printf("Userul a incercat sa stopeze procesul in primele 10 secunde.\n"); }

  /* Se deblochează semnalele CTRL+C și CTRL+Z. */
  printf("\nSe deblocheaza receptia semnalelor ^C si ^Z.\n%s\n\n",
         "Eventualele semnale deja generate de la tastatura, vor fi livrate procesului abia dupa deblocare.\n");

  sigprocmask(SIG_UNBLOCK, &base_mask, NULL);

  /* Un alt interval de așteptare, de 10 secunde. */
  for(i=0; i<10; i++)
  { sleep(1);  printf("Time: %d seconds\n", i+1); }

  printf("\nSfarsitul executiei programului, dupa 20 secunde.\n");
  return 0;
}
