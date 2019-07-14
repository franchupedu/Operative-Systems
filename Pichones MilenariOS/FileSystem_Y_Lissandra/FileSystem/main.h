#ifndef MAIN_H_
#define MAIN_H_

t_log* g_logger;
t_log* error_logger;
t_config * config;

struct metaDataBloques {
	int tamanioBloque;
	int bloques;
	char* magicNumber;
};

struct ConfiguracionFS {
	int puertoEscucha;
	char *puntoDeMontaje; //EMPEZAR Y TERMINA CON "/"
	int retardo;
	int tamanioValue; //en bytes
	int tiempoDump;
};

struct ConfiguracionFS configuracionDePrueba;

char* directorioFileSystem;
char* directorioDeTablas;
char* directorioDeMetadataBloques;
char* directorioDeBloques;
char* directorioArchivoMetadata;

#endif /* MAIN_H_ */
