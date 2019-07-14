#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "Metadata.h"
#include <commons/string.h>


int crearCarpeta(char *directorio) {

	struct stat datosDeCarpeta = { 0 };
	int carpetaExiste = stat(directorio, &datosDeCarpeta);

	if (carpetaExiste == -1) {
		carpetaCreada = mkdir(directorio, 0777);
		if (carpetaCreada == -1) {
			perror("No se pudo crear la carpeta\n");
			return 1;
		}
	}
	return 0;
}

FILE* crearArchivoMetadataDeCarpeta(char* directorio, char* nombreDeArchivo) {

	char* directorioCompletoArchivo = string_from_format("%s%s", directorio,nombreDeArchivo);

	FILE *ArchivoMetadata = fopen(directorioCompletoArchivo, "w+"); //DUDA: los archivos no se escriben an bnario aunque la extensión sea .bin?

	if (!ArchivoMetadata) {
		perror("No se pudo abrir el archivo de escritura");
		//return 1;
		exit(-1);
	}
//	printf("\nArchivo Creado\n");
	return ArchivoMetadata;
}


/*int principal() {
	FILE *ArchivoMetadata;

	struct MetadataFS metaPrueba;
	metaPrueba.block_size = 44;
	metaPrueba.blocks = 9999;
	metaPrueba.magic_number = "LFS";


	crearCarpeta("/home/utnso/workspace/LFS/FileSystem/Metadata");

	ArchivoMetadata = crearArchivoMetadataDeCarpeta("/home/utnso/workspace/LFS/FileSystem/Metadata/", "metadata.bin");


	fwrite(&metaPrueba, sizeof(metaPrueba), 1, ArchivoMetadata);


	struct MetadataFS metaLeido = { 0, 0 };

	fseek(ArchivoMetadata, 0, SEEK_SET);
	int fueLeido;
	fueLeido = fread(&metaLeido, sizeof(metaLeido), sizeof(metaLeido),
			ArchivoMetadata);

	if (!fueLeido) {
		perror("No se pudo leer del archivo\n");
		return 1;
	}

	printf("Datos reales \n TB = %i  B = %i\n Palabra mágica = %s\n",
			metaPrueba.block_size, metaPrueba.blocks, metaPrueba.magic_number);
	printf(
			"\nDatos Leidos \n Tamaño de bloque: %i \n Bloques: %i\n Palabra mágica = %s\n\n",
			metaLeido.block_size, metaLeido.blocks, metaPrueba.magic_number);
	fclose(ArchivoMetadata);
	return 0;

}*/
