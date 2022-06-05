/*
*  Program: sig_ex1.c
*
*  Funcționalitate: realizează o buclă infinită, fără ignorarea semnalelor de întrerupere sau terminare a execuției programului.
*/

#include <stdio.h>

int main() {
	
  printf("Inceput bucla infinita; poate fi oprita apasand tastele CTRL+C sau CTRL+\\ !\n");
  for(;;);
  printf("Sfarsit program.\n");
  return 0;

}
