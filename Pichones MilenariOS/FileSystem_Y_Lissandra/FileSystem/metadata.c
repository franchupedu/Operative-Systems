#include <stdlib.h>
#include <stdio.h>
#include "main.h"

int crearArchivoConfigBloques (FILE* archivoMetadataBloque, t_config** punteroConfigMetadataBloque, char* direccionArchivo){
	t_config* configMetadataBloque = config_create(directorioArchivoMetadata);
	fseek(archivoMetadataBloque, 0, SEEK_SET);
	config_set_value(configMetadataBloque, "BLOCKSIZE", "64");
	config_set_value(configMetadataBloque, "BLOCKS", "100");
	config_set_value(configMetadataBloque, "MAGICNUMBER", "LISSANDRA");
	if(config_save_in_file(configMetadataBloque, direccionArchivo) == -1){
		log_info(error_logger,"No se pudo crear el archivo de configuracion de bloques");
		return -1;
	}
	log_info(g_logger,"Archivo de configuración de los bloques creado en: %s\n", configMetadataBloque->path);
	*punteroConfigMetadataBloque = configMetadataBloque;
	return 0;
}
