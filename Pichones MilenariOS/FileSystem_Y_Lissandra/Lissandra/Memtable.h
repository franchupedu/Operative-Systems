#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <commons/string.h>
#include "../FileSystem/IniciarFileSystem.h"


#ifndef LISSANDRA_MEMTABLE_H_
#define LISSANDRA_MEMTABLE_H_



struct dato {
	u_int timestamp;
	u_int16_t key;
	char* value;
};


struct NodoTabla {
	char* nombreTabla; // valor que contiene el nodo
	t_list* primero;
};

static t_list* listaTablas;
t_config* configuracion;
//extern struct ConfiguracionFS configuracionDePrueba;

int agregarTablaEnMemtable(char* nombreTabla);
bool laTablaSeLlamaAsi(struct NodoTabla* tabla, char* nombreTabla);
struct NodoTabla* tablaEnMemtable(char* nombreTabla);
int agregarRegistroEnLaTablaConTS(char* nombreTabla, u_int16_t key, char* value, u_int timestamp);
int agregarRegistroEnLaTablaSinTS (char* nombreTabla, u_int16_t key, char* value);
bool elRegistroTieneLaKey(struct dato* dato, u_int16_t key);
bool mayorKeyPrimerElemento(struct dato* primero, struct dato* segundo);
struct dato* max_key_lista(t_list* listaRegistros);
struct dato* obtenerRegistroDeTabla(char* tabla, u_int16_t key);
void limpiarRegistrosDeTabla(struct NodoTabla* tabla);
void limpiarTodasLasTablasEnMemtable(t_list* lista);

#endif /* LISSANDRA_MEMTABLE_H_ */
