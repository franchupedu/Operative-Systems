#ifndef FILESYSTEM_METADATA_H_
#define FILESYSTEM_METADATA_H_

struct MetadataFS {
	int block_size;
	int blocks;
	char* magic_number;
};

int carpetaCreada;


int crearCarpeta(char *directorio);
FILE* crearArchivoMetadataDeCarpeta(char* directorio, char* nombreDeArchivo);

#endif /* FILESYSTEM_METADATA_H_ */
