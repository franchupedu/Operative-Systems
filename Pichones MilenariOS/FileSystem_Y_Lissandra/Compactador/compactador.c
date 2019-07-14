#include <commons/config.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "compactador.h"

int existeDumps(char* ubicacion);
void realizarCompactacion();

int ACTIVADO = 1;
char* archivoTmpFlag = "algo";

void *ejecutarCompactador (void* ubicacionDeTabla) {

	printf("\n\n iniciando compactador, Dato recibido: %s \n\n",(char*)ubicacionDeTabla);

	t_config * config;
	char* direccionDeMetadata = string_from_format("%sMetadata",(char *)ubicacionDeTabla);
	//char* direccionDeMetadata = "Metadata";
	config = config_create(direccionDeMetadata);
	//g_logger = log_create("compactadorINFO.log", "LFS", 1, LOG_LEVEL_INFO);
	//error_logger = log_create("compactadorERROR.log", "LFS", 1, LOG_LEVEL_ERROR);

	//configuracionMetadata.consistencia = config_get_int_value(config, "CONSISTENCIA");
	//configuracionMetadata.particiones = config_get_int_value(config, "PARTICIONES");
	configuracionMetadata.tiempoEntreCompactacion = config_get_int_value(config,"TIEMPO_COMPACTACION");
	//configuracionMetadata.tiempoEntreCompactacion = 1000;


	printf("\nPasó por la carga de metadata\nUsa la ruta:%s",direccionDeMetadata);



	while(ACTIVADO){

		if(existeDumps(ubicacionDeTabla)){
			realizarCompactacion(ubicacionDeTabla);
			printf("\n---Realizando compactacion--\n");
		}
		else {
			printf("\n---No requiere compactacion--\n");
		}
		usleep(configuracionMetadata.tiempoEntreCompactacion * 1000);
	}

}

int existeDumps(char* ubicacionDeTabla){
	char* direccion = string_from_format("%s%s.tmp",ubicacionDeTabla,archivoTmpFlag);
	//printf("\n-verificando existencia de dumps en %s\n", direccion);
	struct stat buffer;
	if(stat(direccion,&buffer)==0)
		return 1;
	else
		return 0;
}

void realizarCompactacion(char* ubicacion){
	printf("Se realiza compactacion");
	printf("\n --- Cambiando nombre a los tmp --- \n");
	char* temporalARenombrar = string_from_format("%s%s.tmp", ubicacion, archivoTmpFlag);
	char* temporalNuevoNombre = string_from_format("%sc",temporalARenombrar);
	if(rename(temporalARenombrar, temporalNuevoNombre) == 0)
		printf("Renombrado Con Exito\n");
	else
		printf("Fallado al renombrar\n");
}
