#ifndef DISCO_H_
#define DISCO_H_

#include "estructuras-fs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <time.h>


char* nombre_disco;
int bs;
int count;
size_t tam_disco;
int comienzo_bloques_datos;
int cant_bloques_datos;
int cant_bloques_totales;

typedef enum {
	BORRADO, OCUPADO, DIRECTORIO
} estado_archivo;

typedef struct {
	Gfile nodo;
	int indice;
	int hijo;	//uso este campo para hacer busquedas
} t_metadata_archivo;

Gbloque* disco;
Header* cabecera;
char* bitmap_en_disco;
Gfile* tabla_nodos;
t_bitarray* bitmap;
int tamanio_bitmap_bytes;

t_log* loggerINFO_DISK;
t_log* loggerERROR_DISK;


size_t largo_archivo(char* nombre_archivo);
void iniciar_logs_disco();
void crear_disco();
void formatear();
void hacer_dump();
void crear_raiz();
void mostrar_nodo (Gfile nodo);

Gfile* buscar_archivo(char* path, Gfile* tabla_nodos);
int buscar_indice_nodo_por_path(char* path);
int indice_de_nodo(Gfile* nodo);

int crear_elemento(int estado, char* path, Gfile* tabla_nodos);
int escribir_archivo(char* path, char* contenido, uint32_t tam_archivo, off_t offset);
int leer_archivo(char* path, char** contenido, int cant_leer, off_t offset);
int borrar_archivo(char* path);
int truncar_archivo(char* path, off_t offset);
int renombrar (char *path_viejo, char *path_nuevo);

t_list* listar_archivos_del_directorio(char* path);
int borrar_directorio(char* path);



#endif /* DISCO_H_ */
