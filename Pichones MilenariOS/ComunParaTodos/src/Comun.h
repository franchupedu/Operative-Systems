#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <commons/collections/list.h>


#ifndef COMUN_H_
#define COMUN_H_

//COSAS UTILES A FUTURO-nada por el momento...


typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef enum {
	SC,
	SHC,
	EC
}unaConsistencia;

typedef struct {
	char* nombreDeTabla;
	unaConsistencia consistencia;
	int particiones;
	int tiempoDeCompactacion;
	}metadata;

char* timetamp();
///////////////CLIENTE//////////////////////////
int crear_conexion(char *ip, char* puerto,t_log* log);
void enviar(char* mensaje, int socket_cliente);
void eliminarPaquete(t_paquete* paquete);
void* serializar(t_paquete* paquete, int bytes);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
t_paquete* crearPaquete(void);
void leerPaqueteParaEnviar(int conexion);
void crear_buffer(t_paquete* paquete);
///////////////SERVIDOR//////////////////////////
void* recibir_buffer(int* size, int socket_cliente);
int iniciar_servidor(char* ip, char* puerto, t_log* logger);
int esperar_cliente(int socket_servidor, t_log* logger);
int recibir_operacion(int socket_cliente);
void recibir_mensaje(int socket_cliente, t_log* logger);
t_list* recibir_paquete(int socket_cliente);
//////////////////////////////////////////////////
static void destruirIpMemoria(char *self);
static void destruirMetadata(metadata *self);

#endif /* COMUN_H_ */
