#ifndef SAC_SERVER_ESCUCHAR_PEDIDOS_H_
#define SAC_SERVER_ESCUCHAR_PEDIDOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>
#include "disco.h"


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

struct cliente_op {
	int new_fd;
	struct sockaddr_in their_addr;
};

t_log* loggerINFO;
t_log* loggerERROR;
int puerto;
char* ip;

pthread_mutex_t m_truncar;

pthread_mutex_t thread;

pthread_mutex_t sockete;

void iniciar_logs();
void datos_server();
int largo_archivoB(FILE* archivo);
char* sac_leer(char* path);
void control_error_conexion(int bytes_leidos, struct sockaddr_in cliente,
		int fd_cliente);
void* atender_pedidos(void* cliente_nuevo);

#endif /* SAC_SERVER_ESCUCHAR_PEDIDOS_H_ */
