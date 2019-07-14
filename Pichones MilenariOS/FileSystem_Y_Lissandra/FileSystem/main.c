#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "directorio.h"
#include "metadata.h"

int main() {
	config = config_create("Config.config");
	g_logger = log_create("lfsINFO.log", "LFS", 1, LOG_LEVEL_INFO);
	error_logger = log_create("lfsERROR.log", "LFS", 1, LOG_LEVEL_ERROR);

// ----- Carga de datos de configuracion

	configuracionDePrueba.puertoEscucha = config_get_int_value(config, "Puerto");
	configuracionDePrueba.puntoDeMontaje = config_get_string_value(config, "Punto_Montaje");
	configuracionDePrueba.retardo = config_get_int_value(config,"Retardo");
	configuracionDePrueba.tamanioValue = config_get_int_value(config, "Tamanio_Value");
	configuracionDePrueba.tiempoDump = config_get_int_value(config, "Tiempo_Dump");

// ----- Muestra direccion de montaje

	printf("\n--------------\n\n");
	log_info(g_logger,"punto de montaje: %s\n", configuracionDePrueba.puntoDeMontaje);

// -----CREAR CARPETAS FS

	directorioFileSystem = crearDirectorio(configuracionDePrueba.puntoDeMontaje, "FileSystem");
	directorioDeTablas = crearDirectorio(directorioFileSystem, "Tablas");
	directorioDeMetadataBloques = crearDirectorio(directorioFileSystem, "Metadata");
	directorioDeBloques = crearDirectorio(directorioFileSystem, "Bloques");

//----- CREAR METADATA DE BLOQUES

	directorioArchivoMetadata = string_from_format( "%s%s", directorioDeMetadataBloques, "metadata.bin");
	FILE* archivoMetadataBloque = crearArchivo(directorioArchivoMetadata);
	if (archivoMetadataBloque == NULL) {
		exit(-1);
	}
	log_info(g_logger,"Directorio del archivo metadata de Bloques : %s\n", directorioArchivoMetadata);
	t_config * metadataBloque;
	metadataBloque->path = "ESTO NO DEBERIA ESTAR";
	if(crearArchivoConfigBloques (archivoMetadataBloque, &metadataBloque, directorioArchivoMetadata) == -1){
		printf("ERROR");
		exit(-1);
	}

	printf("\n --- FLAG 1 ---\n");
	printf(" - path metadataBloque : %s",metadataBloque->path);

	/*
	struct metaDataBloques metaBloquesPrueba;

	metaBloquesPrueba.tamanioBloque = config_get_int_value(metadataBloques, "BLOCKSIZE");
	metaBloquesPrueba.bloques = config_get_int_value(metadataBloques, "BLOCKS");
	metaBloquesPrueba.magicNumber = config_get_string_value(metadataBloques, "MAGICNUMBER");

	printf("\n --- FLAG 2 ---\n");

//------------------------------------------------------------------------------------------CREAR BLOQUES
	const int cantBloques = metaBloquesPrueba.bloques;

	for(int cantBloques = metaBloquesPrueba.bloques; cantBloques > 0; cantBloques --) {
		char* nombreBloque = string_from_format("%sbloque%i.bin", directorioDeBloques, cantBloques);
		crearArchivo(nombreBloque);
	}

	log_info(g_logger,"**Se han creado %d bloques** \n",cantBloques);


	printf("\n --- FLAG 3 ---\n");

//----- CREAR BITMAP

	t_bitarray *bitmap;
	FILE* archivoBitmap;
	char bitarreglo[cantBloques];

	memset(bitarreglo, 0, cantBloques);
	bitmap = bitarray_create_with_mode(bitarreglo, cantBloques, LSB_FIRST);

	char* directorioArchivoBITMAP = string_from_format("%s%s",directorioDeMetadataBloques, "bitmap.bin");
	log_info(g_logger,"Directorio del archivo bitmap de los bloques: %s \n", directorioArchivoBITMAP);

	archivoBitmap = fopen(directorioArchivoBITMAP, "w+b");
	if (!archivoBitmap)
		log_info(error_logger,"No se pudo abrir el archivo de escritura bitmap\n");

	if (fwrite(bitmap,sizeof(t_bitarray), 1, archivoBitmap))
		log_info(g_logger,"bitmap creado en %s",directorioArchivoBITMAP);
		*/
/*
//------ TABLAS
	char* operacion;
	printf("Ingrese operacion: ");
	scanf("%s",operacion);
	ingresarTabla();

	printf("\n---Ahora duerme 30 segundos---\n");
	sleep(30);

	//smostrarMetadataDeTODASlasTablas();
	printf("\n-------\n");
	//mostrarMetadataDeUNAtabla("fran");
	//smostrarMetadataDeUNAtabla("frannnnnnn");

	printf("Existe la tabla fran : %i\n----\n", existeTabla("fran"));
	printf("Existe la tabla franchute : %i\n----\n", existeTabla("franchute"));
*/
	//fclose(archivoBitmap);
	fclose(archivoMetadataBloque);

	return 0;
}
