#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Metadata.h"
#include "IniciarFileSystem.h"
#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/string.h>


int main() {

	t_config * config;
	config = config_create("Config.config");

	configuracionDePrueba.puertoEscucha = config_get_int_value(config, "Puerto");
	configuracionDePrueba.puntoDeMontaje = config_get_string_value(config, "Punto_Montaje");
	configuracionDePrueba.retardo = config_get_int_value(config,"Retardo");
	configuracionDePrueba.tamanioValue = config_get_int_value(config, "Tamanio_Value");
	configuracionDePrueba.tiempoDump = config_get_int_value(config, "Tiempo_Dump");

	printf("punto de montaje: %s\n", configuracionDePrueba.puntoDeMontaje);

	char* directorioFileSystem = string_from_format("%sFileSystem/", configuracionDePrueba.puntoDeMontaje);
	printf("directorio fs : %s\n", directorioFileSystem);
	crearCarpeta(directorioFileSystem);

	char* directorioDeTablas = string_from_format("%sTablas/", directorioFileSystem);
	crearCarpeta(directorioDeTablas);

	char* directorioDeMetadataBloques = string_from_format("%sMetadata/", directorioFileSystem);
	crearCarpeta(directorioDeMetadataBloques);

	char* directorioDeBloques = string_from_format("%sBloques/", directorioFileSystem);
	crearCarpeta(directorioDeBloques);


	FILE* metadataBloque;
	char* metadataBin = "metadata.bin";
	metadataBloque = crearArchivoMetadataDeCarpeta(directorioDeMetadataBloques, metadataBin);

	//Leer archivo de metadata de bloques ----- Proximamente...


	char contenidoMetadataBloques[] = "BLOCKSIZE=64\nBLOCKS=100\nMAGICNUMBER=LISSANDRA";

	fwrite(contenidoMetadataBloques,48,1, metadataBloque);

	char* archivoMetadataDirectorio = string_from_format("%s%s", directorioDeMetadataBloques, metadataBin);


	//ACÁ EMPIEZA EL PROBLEMA **************************************


	t_config * metadataBloques;
	metadataBloques = config_create(archivoMetadataDirectorio);
	printf("el config creado con el path: %s\n", metadataBloques->path);

	struct metaDataBloques metaBloquesPrueba;

	//metaBloquesPrueba.tamanioBloque =
		printf("Block size de la metadata :  %i\n"	,config_get_int_value(metadataBloques, "BLOCKSIZE"));
/*	metaBloquesPrueba.bloques = config_get_int_value(metadataBloques, "BLOCKS");
	metaBloquesPrueba.magicNumber = config_get_string_value(metadataBloques, "MAGIC_NUMBER");


	printf("metadataBloquesPureba\n bloques:%i\ntamanioBloques:%i\nmagicNuber:%s\n",metaBloquesPrueba.bloques, metaBloquesPrueba.tamanioBloque,metaBloquesPrueba.magicNumber);

	int cantBloques = metaBloquesPrueba.bloques;
	/*char nombreBloque[50];
	for(int cantBloques = metaBloquesPrueba.bloques; cantBloques > 0; cantBloques --) {
		sprintf(nombreBloque, "bloque%i.bin", cantBloques);
		crearArchivoMetadataDeCarpeta(directorioDeBloques, nombreBloque);
	}

	printf("**Se han creado %d bloques** /n",cantBloques);
	//crear bitmap
	char* bitarreglo;
	struct t_bitarray *bitmap;
	FILE* archivoBitmap;

	bitarreglo = malloc(metaBloquesPrueba.bloques);
	bitmap = bitarray_create(bitarreglo, sizeof(bitarreglo));

	archivoBitmap = crearArchivoMetadataDeCarpeta(directorioDeMetadataBloques, "bitmap.bin");


	fwrite(bitmap,sizeof(bitmap), 1, archivoBitmap);

	fclose(archivoBitmap);
	fclose(metadataBloque);
*/
	return 0;
}
