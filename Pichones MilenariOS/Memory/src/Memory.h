#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <readline/readline.h>
#include "../../ComunParaTodos/src/Comun.c"
#include <commons/collections/list.h>
#include <stdbool.h>
#include <pthread.h>

#ifndef MEMORY_H_
#define MEMORY_H_

typedef struct{
	uint32_t timestamp;
	uint16_t key;
	char value[];
}registro;

typedef struct {
	int numeroDePagina;
	bool modificado;
	int posicionRegistro;
	/*Los registros tienen que estar en un malloc general para asi si se cae una memoria no pase nada.
	 * Tambien por que quisas mas de una memoria tienen datos de ese registro en particular (INCONSISTENCIAS EVERIWERE)
	 * memeDePerritoEnFuego.jpg*/
}pagina;

typedef struct {
	char* nombreDeTabla;
	t_list *tablaDePaginas;
}segmento;


typedef struct {
	char* Ip;
	char* puerto;
}paraConectar;

//Variables Globales
t_log* g_logger;
t_config* g_config;
t_list* registrosTotales;
//Cliente
int crearConexion(char *ip, char* puerto,t_log* log);


//Servidor
int iniciarServidor(char* ip, char* puerto);
int esperarCliente(int socket_servidor);
int cambiarUnCharASuNumero(char unCaracter);
int recibirOperacion(int socket_cliente);
int recibirTamanio();


//Requests
void hacerSelect();
void hacerInsert();
void hacerCreate();
void hacerDescribe();
void hacerDrop();
void hacerJournal();

#endif /* MEMORY_H_ */
