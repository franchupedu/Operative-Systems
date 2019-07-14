#ifndef DIRECTORIO_H_
#define DIRECTORIO_H_

#include "directorio.c"
#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>
#include <commons/log.h>


struct MetadataFS {
	int block_size;
	int blocks;
	char* magic_number;
};

int carpetaCreada;
extern t_log* g_logger;
extern t_log* error_logger;

int crearCarpeta(char *directorio);
/**
	* @NAME: crearArchivoMetadataDeCarpeta
	* @DESC: Crea un archivo en un directorio específico y devuelve el FD
*/

char* crearDirectorio(char* ubicacion, char* nombreCarpeta);

FILE* crearArchivo(char* ubicacion);

#endif /* DIRECTORIO_H_ */
