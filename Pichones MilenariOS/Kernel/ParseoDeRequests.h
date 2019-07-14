#include "Definiciones.h"

#ifndef KERNEL_PARSEODEREQUESTS_H_
#define KERNEL_PARSEODEREQUESTS_H_

// Funciones que parsean requests.
int esalfanumerico (char caracter);
int patsearselect (char *palabra);
int parsearinsert (char *palabra);
int esdeaceptacionconsistencia (int actual);
int parsearcreate (char *palabra);
int parseardescribe (char *palabra);
int parseardrop (char *palabra);
int parsearjournal (char *palabra);
int parsearadd (char *palabra);
int parsearrun (char *palabra);
int parsearmetrics (char *palabra);
int esdeaceptacion (int estado);
int parsear (char *palabra);

#endif
