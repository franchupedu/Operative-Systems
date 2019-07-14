#include <sys/stat.h>
#include "main.h"

int crearCarpeta(char *directorio) {

	struct stat datosDeCarpeta;

	if (stat(directorio, &datosDeCarpeta) != 0) {
		log_info(g_logger,"Creando carpeta en: %s", directorio);
		if (mkdir(directorio, 0777) == 0) {
			log_info(g_logger,"Carpeta creada en: %s\n", directorio);
			return 1;//La carpeta no existía y fue creada
		}
		else {
			log_info(error_logger,"No se pudo crear la carpeta en: %s\n", directorio);
			return -1;//La carpeta no pudo ser creada
		}
	}
	else {
		log_info(g_logger,"Carpeta ya existente en: %s\n", directorio);//La carpeta ya existía
		return 0;
	}
}

char* crearDirectorio(char* ubicacion, char* nombreCarpeta){
	char* nuevoDirectorio = string_from_format("%s%s/", ubicacion, nombreCarpeta);
	log_info(g_logger,"Creando directorio en: %s", nuevoDirectorio);
	switch(crearCarpeta(nuevoDirectorio))
	{
		case 0:
			log_info(g_logger,"Directorio ya existente en: %s\n", nuevoDirectorio);
			return nuevoDirectorio;
		case 1:
			log_info(g_logger,"Directorio creado en: %s\n", nuevoDirectorio);
			return nuevoDirectorio;
		default:
			log_info(error_logger,"No se pudo crear el directorio en: %s\n", nuevoDirectorio);
			return NULL;
	}
}

FILE* crearArchivo(char* ubicacion) {
	FILE* archivo = fopen(ubicacion, "w+");
	if (archivo == NULL)
		log_info(error_logger,"No se pudo abrir el archivo de escritura");
	//log_info(g_logger,"%s creado en %s \n", nombreDeArchivo, directorioCompletoArchivo);
	return archivo;
}
