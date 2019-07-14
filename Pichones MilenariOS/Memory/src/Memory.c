
#include "Memory.h"


// Función que devuelve el número base elevado al número exponente.
int potencia (int base, int exponente) {
	if (! exponente)
		return 1;

	for (int i = 1; i < exponente; i++)
		base *= base;

	return base;
}

// Función que castea un stream de números a un int.
int castear (char *numero) {
	int numerocasteado = 0;

	for (int i = 0; i < strlen (numero); i++)
		numerocasteado += (numero [i] - 48) * potencia (10, strlen (numero) - i - 1);

	return numerocasteado;
}

typedef enum{
	SELECT=6,
	INSERT=12,
	CREATE=18,
	DESCRIBE=26,
	DROP=29,
	JOURNAL=36,
}codigoRequest;

int crearConexion(char *ip, char* puerto,t_log* log)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	//Implemente el while ya que sino moria al no poder conectar con el servidor
	while(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		log_error(log, "No se pudo conectar, intentando nuevamente\n");
		sleep(10);//es para que no consuma casi constantemente los recursos
	}
	log_error(log, "Conexion realizada\n");

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciarServidor(char* ip, char* puerto)
{


	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    int cosita=1;
    int* cosa = &cosita;
    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
        	setsockopt(socket_servidor,SOL_SOCKET,SO_REUSEADDR,cosa, sizeof(cosa));
			continue;
    }
        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    log_trace(g_logger, "Listo para escuchar a mi cliente");

    return socket_servidor;
}

int esperarCliente(int socket_servidor){
	struct sockaddr_in dir_cliente;
	unsigned int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	log_info(g_logger, "Se conecto un cliente!");

	printf("paso1\n");

	return socket_cliente;
}

int cambiarUnCharASuNumero(char unCaracter){
	log_info(g_logger, "unCaracter %c", unCaracter);
	switch(unCaracter){
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
			return 2;
	case '3':
			return 3;
	case '4':
			return 4;
	case '5':
			return 5;
	case '6':
			return 6;
	case '7':
			return 7;
	case '8':
			return 8;
	case '9':
			return 9;
	default:
			return -1;
	}
}
int algo(int codigoEnEntero, int socket_cliente){
	char cod_op='e';
	switch(codigoEnEntero){
		case 6:
			return codigoEnEntero;
		case 1:

			if(recv(socket_cliente, &cod_op, sizeof(char), 0) != 0){
				codigoEnEntero=codigoEnEntero*10+cambiarUnCharASuNumero(cod_op);
				return codigoEnEntero;
			}
			else
			{
				close(socket_cliente);
				return -1;
			}
		case 2:
			if(recv(socket_cliente, &cod_op, sizeof(char), 0) != 0){
				codigoEnEntero=codigoEnEntero*10+cambiarUnCharASuNumero(cod_op);
				return codigoEnEntero;
			}
			else
			{
				close(socket_cliente);
				return -1;
			}
		case 3:
			if(recv(socket_cliente, &cod_op, sizeof(char), 0) != 0){
				codigoEnEntero=codigoEnEntero*10+cambiarUnCharASuNumero(cod_op);
				return codigoEnEntero;
			}
			else
			{
				close(socket_cliente);
				return -1;
			}
		default:
			return -3;
		}
	return -2;
}
int recibirOperacion(int socket_cliente){
	char cod_op='e';
	if(recv(socket_cliente, &cod_op, sizeof(char), 0) != 0){
		int codigoEnEntero=cambiarUnCharASuNumero(cod_op);
		log_info(g_logger, "codigoEnEntero: %i", codigoEnEntero);
		return algo(codigoEnEntero,socket_cliente);
	}
	else
	{
		close(socket_cliente);
		return -1;
	}
}

int recibirTamanio(int socket_cliente)
{
	char tamanio='e';
	if(recv(socket_cliente, &tamanio, sizeof(char), 0) != 0){
		log_info(g_logger, "tamanio recibido %c\n", tamanio);
		return tamanio;
	}
	else
	{
		close(socket_cliente);
		return -1;
	}
}

char* recibirRequest(int tamanio, int socket_cliente){
	char* request;
		if(recv(socket_cliente, &request, tamanio, 0) != 0)
			return request;
		else
		{
			close(socket_cliente);
			log_error(g_logger, "No se recibi bien el mensaje esperado");
			return "Error";
		}
}

void hacerSelect(int tamanio, int socket_cliente){
	printf("hacerSelect. Tamanio:  %i\n", tamanio);
}
void hacerInsert(int tamanio, int socket_cliente){
	printf("hacerInsert");
}
void hacerCreate(int tamanio, int socket_cliente){
	printf("hacerCreate");
}
void hacerDescribe(int tamanio, int socket_cliente){
	printf("hacerDescribe");
}
void hacerDrop(int tamanio, int socket_cliente){
	printf("hacerDrop");
}
void hacerJournal(){
	printf("hacerJournal");
}

void ejecutarCliente(paraConectar *estructDeConeccion){

	char* IpFileSystem=estructDeConeccion->Ip;
	char* puertoFileSystem=estructDeConeccion->puerto;

	int conexion=crearConexion(IpFileSystem,puertoFileSystem,g_logger);
	//Ver bien que hay que enviar
	enviar("Me conecte bien \n",conexion);

	leerPaqueteParaEnviar(conexion);
}

void ejecutarServidor(paraConectar *estructDeConeccion){

	char* IpMemoria=estructDeConeccion->Ip;
	char* puertoMemoria=estructDeConeccion->puerto;

	int servidor = iniciarServidor(IpMemoria,puertoMemoria);
	int cliente = esperarCliente(servidor);
	printf("esta\n");
	bool seguimos=true;
	while(seguimos)
	{
		printf("esta2\n");
		char cod_op = recibirOperacion(cliente);
		int tamanioRequest=0;
		log_info(g_logger, "codigo operacion: %i \n", cod_op);
		switch(cod_op)
		{
		case SELECT:
			hacerSelect(tamanioRequest,cliente);
			tamanioRequest=recibirTamanio(cliente);
			hacerSelect(tamanioRequest,cliente);
			break;
		case INSERT:
			tamanioRequest=recibirTamanio(cliente);
			hacerInsert(tamanioRequest,cliente);
			break;
		case CREATE:
			tamanioRequest=recibirTamanio(cliente);
			hacerCreate(tamanioRequest,cliente);
			break;
		case DESCRIBE:
			tamanioRequest=recibirTamanio(cliente);
			hacerDescribe(tamanioRequest,cliente);
			break;
		case DROP:
			tamanioRequest=recibirTamanio(cliente);
			hacerDrop(tamanioRequest,cliente);
			break;
		case JOURNAL:
			hacerJournal();
			break;
		case 'E':
			log_error(g_logger, "¿Que que paso? Se desconectaron. ¿Por que? No hay poque -.-");
			seguimos=false;
			break;

		default:
			log_warning(g_logger, "No es ninguno de los codigos que me dijiste, ojo vieja. Proba de nuevo");
			break;
		}
	}
}



int main()
{/*
	//char=int va bien, int=char rompe
	int cosa=98;
	char cosa2=cosa;
	int cosa3 = cosa2;
	printf("Cosa i: %i\n", cosa);
	printf("Cosa2 c: %c\n", cosa2);
	printf("Cosa3 i: %i\n", cosa3);
*/
	//Estructura .log
	g_logger=log_create("Memory.log", "Memory", 1, LOG_LEVEL_INFO);
	log_info(g_logger, "Log creado\n");
	//Estructura .config
	g_config=config_create("Memory.config");
	log_info(g_logger, "Config recibido\n");

	//Levantar datos del config

	char* puertoMemoria=config_get_string_value(g_config, "PUERTO");
	log_info(g_logger, "Puerto de memoria para conexion: %s \n", puertoMemoria);

	char* IpMemoria=config_get_string_value(g_config, "IP_ME");
	log_info(g_logger, "IP Memoria para conexion: %s \n", IpMemoria);

	char* IpFileSystem=config_get_string_value(g_config, "IP_FS");
	log_info(g_logger, "IP File System para conexion: %s \n", IpFileSystem);

	char* puertoFileSystem=config_get_string_value(g_config, "PUERTO_FS");
	log_info(g_logger, "Puerto File System para conexion: %s \n", puertoFileSystem);

	char** IpsSeeds=config_get_array_value(g_config, "IP_SEEDS");

	bool mostrarIP=(IpsSeeds[0]!=NULL);
	int ipSeedMostrar =0;
	while(mostrarIP){
		log_info(g_logger, "IP de la seed %d es: %s \n",ipSeedMostrar, IpsSeeds[ipSeedMostrar]);
		ipSeedMostrar++;
		if (IpsSeeds[ipSeedMostrar]==NULL){
			mostrarIP=false;
		}
	};

	char** puertosSeeds=config_get_array_value(g_config, "PUERTO_SEEDS");

	bool mostrarPuerto=(puertosSeeds[0]!=NULL);
	int puertoSeedMostrar=0;
	while(mostrarPuerto){
		log_info(g_logger, "Puestos de la seed %d de memoria: %s \n", puertoSeedMostrar, puertosSeeds[puertoSeedMostrar]);
		puertoSeedMostrar++;
		if (puertosSeeds[puertoSeedMostrar]==NULL){
			mostrarPuerto=false;
		}
	};


	int retardoDeMemoria=config_get_int_value(g_config, "RETARDO_MEM");
	log_info(g_logger, "Retardo de memoria: %d \n", retardoDeMemoria);

	int retardoDeFileSystem=config_get_int_value(g_config, "RETARDO_FS");
	log_info(g_logger, "Retardo de File System: %d \n", retardoDeFileSystem);

	int tamanioDeMemoria=config_get_int_value(g_config, "TAM_MEM");
	log_info(g_logger, "Tamanio de la memoria: %d \n", tamanioDeMemoria);

	int retardoDeJournal=config_get_int_value(g_config, "RETARDO_JOURNAL");
	log_info(g_logger, "Retardo de Journal: %d \n", retardoDeJournal);

	int retardoDeGosssiping=config_get_int_value(g_config, "RETARDO_GOSSIPING");
	log_info(g_logger, "Retardo de Gossiping: %d \n", retardoDeGosssiping);

	int numeroDeMemoria=config_get_int_value(g_config, "MEMORY_NUMBER");
	log_info(g_logger, "Numero de memoria: %d \n", numeroDeMemoria);

	registrosTotales = malloc(tamanioDeMemoria);


	paraConectar *conectCliente=malloc(sizeof(paraConectar));
	conectCliente->Ip=IpFileSystem;
	conectCliente->puerto=puertoFileSystem;

	paraConectar *conectServidor=malloc(sizeof(paraConectar));
	conectServidor->Ip=IpMemoria;
	conectServidor->puerto=puertoMemoria;

	pthread_t hiloServidor;
//	pthread_t hiloCliente;

	//pthread_create( &hiloCliente, NULL,(void* ) ejecutarCliente, (void*) conectCliente);
	pthread_create( &hiloServidor, NULL,(void* ) ejecutarServidor, (void*) conectServidor);

	//pthread_join(hiloCliente,NULL);
	pthread_join(hiloServidor,NULL);

//libero memoria


	free(conectCliente);
	free(conectServidor);
	//

	bool liberarIP=(IpsSeeds[0]!= NULL);
	int ipSeedliberar =0;
	while(liberarIP){
		free(IpsSeeds[ipSeedliberar]);
		ipSeedliberar++;
		if (IpsSeeds[ipSeedliberar]== NULL){
			free(IpsSeeds);
			liberarIP=false;
		}
	};

	bool liberarPuerto=(puertosSeeds[0]!=NULL);
	int puertoSeedliberar=0;
	while(liberarPuerto){
		free(puertosSeeds[puertoSeedliberar]);
		puertoSeedliberar++;
		if (puertosSeeds[puertoSeedliberar]== NULL){
			free(puertosSeeds);
			liberarPuerto=false;
		}
	};

	//
	config_destroy(g_config);
	log_destroy(g_logger);

	return EXIT_SUCCESS;
}
