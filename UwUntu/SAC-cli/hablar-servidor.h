#ifndef SAC_CLI_HABLAR_SERVIDOR_H_
#define SAC_CLI_HABLAR_SERVIDOR_H_

#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define DESCRIBIR 24

#define ABRIR_DIRECTORIO 23
#define ELIMINAR_DIRECTORIO 22
#define LISTAR_DIRECTORIO 21
#define CREAR_NODO 20

#define ABRIR_ARCHIVO 8
#define ELIMINAR_ARCHIVO 6
#define LEER_ARCHIVO 5
#define ESCRIBIR_ARCHIVO 4

#define TRUNCAR 100
#define RENOMBRAR 420

t_config* config;
t_log* loggerINFO;
t_log* loggerERROR;

char ip_server[20];
int puerto_server;
void datos_server();
char* pedir_oper_sacServer(const char* pedido_oper, int codigo_oper);
char* sac_leer(const char* path);
char* sac_crear_nodo(const char* path);
char* sac_abrir(const char* path);
char* sac_abrir_directorio(const char* path);
char* sac_escribir(const char* path);
char* sac_describir(const char* path);
char* sac_listar (const char* path);
char *sac_truncar (const char *path);
char *sac_borrar_archivo (const char *path);
char *sac_borrar_directorio (const char *path);
char *sac_renombrar (const char *path);

pthread_mutex_t sem_recv;

pthread_mutex_t sockete;

#endif /* SAC_CLI_HABLAR_SERVIDOR_H_ */
