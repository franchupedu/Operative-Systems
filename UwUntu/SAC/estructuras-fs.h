#ifndef SAC_CLI_ESTRUCTURAS_FS_H_
#define SAC_CLI_ESTRUCTURAS_FS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 4096
#define CANT_INDIR_SIMPLE 1000
#define CANT_DIRECTOS 1024
#define MAX_FILE_COUNT 1024
#define MAX_FILE_NAME_LEN 71
#define IDENTIFICADOR_SAC "SAC"
#define BLOQUES_SIEMPRE_OCUPADOS 1026


typedef struct {
	char tam_bloque[BLOCK_SIZE];
} Gbloque;

typedef uint32_t ptrGBloque;

typedef struct {
	unsigned char identificador[3];
	uint32_t version;
	ptrGBloque inicio_bitmap;
	uint32_t bloques_bitmap;
	unsigned char relleno[4081]; //hacer un memset -- con relleno = 4080, header pesa 4096B
						//con relleno = 4081, header pesa 4100B ?????
} Header;

typedef struct {   //1 nodo = 1 bloque
	uint8_t estado;
	char nombre_archivo[MAX_FILE_NAME_LEN];
	ptrGBloque bloque_padre;
	uint32_t tam_archivo;
	uint64_t fecha_creacion;
	uint64_t fecha_modificacion;
	ptrGBloque simple_indirect[CANT_INDIR_SIMPLE];
} Gfile;


typedef struct { //4096 bytes
	ptrGBloque ptr_direct[CANT_DIRECTOS];
} Bloque_ptr;




#endif /* SAC_CLI_ESTRUCTURAS_FS_H_ */
