#include "ParseoDeRequests.c"

#ifndef KERNEL_EJECUCIONDEREQUESTS_H_
#define KERNEL_EJECUCIONDEREQUESTS_H_

// Funciones que ejecutan requests.
void ejecutarselect (char *request, int tamaniorequest);
void ejecutarinsert (char *request, int tamaniorequest);
void ejecutarcreate (char *request, int tamaniorequest);
void ejecutardescribe (char *request, int tamaniorequest);
void ejecutardrop (char *request, int tamaniorequest);
void ejecutarjournal ();
int potencia (int base, int exponente);
int castear (char *numero);
void ejecutaradd (char *request);
void sacarproceso (struct PCB pcb);
void enviaraready (struct PCB pcb);
void ejecutarrequest (char *request);
void ejecutararchivo (struct PCB pcb);
void ejecutarrun (char *request);
void ejecutarmetrics ();

#endif
