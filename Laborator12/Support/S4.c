/*
*  Program: sig_ex5.c
*
*  Funcționalitate: ilustrează modul de așteptare a unui anumit tip de semnal.
*/

#include <stdio.h>
#include <signal.h>

int main()
{
  sigset_t mask;

  /* Stabilește masca de semnale ce vor fi blocate: toate semnalele, exceptând SIGQUIT (și tipurile de semnale neblocabile) */
  sigfillset(&mask);
  sigdelset (&mask, SIGQUIT);

  printf("Suspendarea executiei programului pana la apasarea tastelor CTRL+\\.\n");

  /* Se suspendă execuția programului, în așteptarea sosirii unui semnal SIGQUIT, restul semnalelor fiind blocate (cu excepția celor neblocabile) */
  sigsuspend(&mask);

  printf("Sfarsit program.\n");
  return 0;
}


//sigfillset(mask) - seteaza toate semnalele ca blocate si le pune in masca
//sigemptyset(mask) - pune o masca cu toate semnalele neblocate
//sigaddset(mask, signal) - seteazaza signalul signal pe 1 in maska
//sigdelset(mask, signal) - sterge signalul signal din maska 
//sigsuspend(mask) - inlocuieste temporar masca de semnale cu o masca temporara. Suspenda executia
//pana cand un semnal neblocat din masca este trimis

//sigprocmask(SIG_BLOCK/SIG_UNBLOCK/SIG_SETMASK, maskaCuSemnale, NULL);
//se seteaza semnalele blocate ca fiind cele din maska
//daca punem in loc de null o alta variabila masca, in ea se va returna vechea masca de semnale (cea dinainte de functie)

