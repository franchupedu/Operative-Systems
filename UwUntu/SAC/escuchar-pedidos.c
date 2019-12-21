#include "escuchar-pedidos.h"

extern Gfile* tabla_nodos;
extern estado_archivo estado;


void iniciar_logs() {

	loggerINFO = log_create("logs-serverINFO.log", "SAC-SERVER", 1,
			LOG_LEVEL_INFO);
	loggerERROR = log_create("logs-serverERROR.log", "SAC-SERVER", 1,
			LOG_LEVEL_ERROR);
}


void datos_server() {

	t_config* config = config_create("config-server.config");
	puerto = config_get_int_value(config, "PUERTO");
	ip = strdup(config_get_string_value(config, "IP"));
	config_destroy(config);
}


void control_error_conexion(int bytes_leidos, struct sockaddr_in cliente,
		int fd_cliente) {

	if (bytes_leidos == 0) {
		log_info(loggerERROR, "Se desconecto el cliente %s\n de %i\n",
				inet_ntoa(cliente.sin_addr), fd_cliente);
		pthread_exit(NULL);
	}
	if (bytes_leidos < 0) {
		log_info(loggerERROR, "recv");
		pthread_exit(NULL);
	}
}


int largo_archivoB(FILE* archivo) {

	int largo;
	fseek(archivo, 0, SEEK_END);
	largo = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	log_info(loggerINFO, "\nlargo_archivo: %i\n", largo);

	return largo;
}


void* atender_pedidos(void* cliente_nuevo) {

	log_info(loggerINFO, "\n ~~~~ Hola desde el hilo %ld ~~~~ \n",
			pthread_self());

	struct cliente_op datos_cliente;
	datos_cliente.new_fd = ((struct cliente_op*) cliente_nuevo)->new_fd;
	datos_cliente.their_addr = ((struct cliente_op*) cliente_nuevo)->their_addr;
	//Hago esto para que el hilo no use una variable  comprtida
	//así tiene su propia estructura



	char* mensajeCliente;
	int nbytesTAM;
	int nbytesMSJ;
	int tamanioMSJ;
	int codigo_operacion;
	int tipo_nodo;

	//while (1) {

		//Rcv Código de operación
		nbytesTAM = recv(datos_cliente.new_fd, &codigo_operacion, sizeof(int), MSG_WAITALL);

		control_error_conexion(nbytesTAM, datos_cliente.their_addr,
				datos_cliente.new_fd);


		log_info(loggerINFO, "codigo de la operación: %i\n", codigo_operacion);

		//Rcv Tamaño del mensaje del cliente
		nbytesTAM = recv(datos_cliente.new_fd, &tamanioMSJ, sizeof(int), 0);
		control_error_conexion(nbytesTAM, datos_cliente.their_addr,
				datos_cliente.new_fd);

		//Rcv Mensaje del cliente
		mensajeCliente = malloc(tamanioMSJ + 1);
		nbytesMSJ = recv(datos_cliente.new_fd, mensajeCliente, tamanioMSJ, 0);
		log_info(loggerINFO, "bytes del mensaje recibido %i\n", nbytesMSJ);
		control_error_conexion(nbytesMSJ, datos_cliente.their_addr,
				datos_cliente.new_fd);
		mensajeCliente[tamanioMSJ] = '\0';

		log_info(loggerINFO, "%s dice >>> %s \n",
				inet_ntoa(datos_cliente.their_addr.sin_addr), mensajeCliente);

		char* lo_que_quiere;
		int nodo;

		// ------------------------------------------------------------------------------


		int tam_path;
		char* path;
		int tam_buf;
		char* buf;
		int offset;
		int size;
		int estado;
		int tam_a_enviar = 0;
		Gfile* directorio;

		switch (codigo_operacion) {

		case RENOMBRAR:

			log_info (loggerINFO, "El cliente quiere renombrar un archivo\n");

			memcpy (&tam_buf, mensajeCliente, sizeof (int));
			memcpy (&tam_path, mensajeCliente + sizeof (int), sizeof (int));
			buf = malloc (tam_buf + 1);
			path = malloc (tam_path + 1);
			memcpy (buf, mensajeCliente + 2 * sizeof (int), tam_buf);
			memcpy (path, mensajeCliente + 2 * sizeof (int) + tam_buf, tam_path);
			buf [tam_buf] = '\0';
			path [tam_path] = '\0';
			estado = renombrar (buf, path);
			lo_que_quiere = malloc (sizeof (int));
			memcpy (lo_que_quiere, &estado, sizeof (int));
			tam_a_enviar = sizeof (int);
			break;

		case LEER_ARCHIVO:

			log_info(loggerINFO, "El cliente quiere leer\n");

			memcpy(&tam_path, mensajeCliente, sizeof(int));
			path = malloc(tam_path + 1);
			memcpy(path, mensajeCliente + sizeof(int), tam_path);
			memcpy(&size, mensajeCliente + sizeof (int) + tam_path, sizeof (int));
			memcpy(&offset, mensajeCliente + sizeof (int) + tam_path + sizeof (int), sizeof (int));


			printf(">>>>>>>>>>>> tam a leer: %i offset: %i\n***\n", size, offset);
			printf(">>>>>>>  path: %s\n***\n", path);
			path[tam_path] = '\0';

			char* leido;  //Reservar tamaño?
			int tam_leido = leer_archivo(path, &leido, size, offset);

			free (path);
			if (tam_leido == -1) {

				lo_que_quiere = malloc (sizeof (int));
				memcpy (lo_que_quiere, &tam_leido, sizeof (int));
				tam_a_enviar = sizeof (int);
				break;
			}


			printf(">>>>>>>>>>>> tam leido: %i\n***\n", tam_leido);
			printf("dirección del disco: %p\n", disco);

			lo_que_quiere = malloc(tam_leido + sizeof (int));
			memcpy(lo_que_quiere, &tam_leido, sizeof(int));
			printf("Este malloc no me sale \n");
			memcpy(lo_que_quiere + sizeof(int), leido, tam_leido);
			//lo_que_quiere [sizeof (int) + tam_leido] = '\0';
			tam_a_enviar = sizeof(int) + tam_leido;

			printf("Tam a enviar en LEER_ARCHIVO: %i\n", tam_a_enviar);
			break;

		case DESCRIBIR:

			log_info(loggerINFO, "El cliente quiere describir un directorio.");

			Gfile* Gnodo_archivo = buscar_archivo(mensajeCliente, tabla_nodos);

			if (Gnodo_archivo == NULL) {
				lo_que_quiere = malloc(sizeof(int));
				estado = 1;
				memcpy(lo_que_quiere, &estado, sizeof(int));

				tam_a_enviar += sizeof(int);
				break;

			}
			if (Gnodo_archivo -> estado == DIRECTORIO)
				estado = 2;

			if (Gnodo_archivo->estado == OCUPADO)
				estado = 0;

				lo_que_quiere = malloc(2*sizeof(int) + 2*sizeof(uint64_t));
				// estado + fecha C + fecha M + tam
				memcpy(lo_que_quiere, &estado, sizeof(int));
				memcpy(lo_que_quiere + sizeof(int), &(Gnodo_archivo->fecha_creacion), sizeof(uint64_t));
				memcpy(lo_que_quiere + sizeof(int) + sizeof(uint64_t), &(Gnodo_archivo->fecha_modificacion), sizeof(uint64_t));
				memcpy(lo_que_quiere + sizeof(int) + 2*sizeof(uint64_t), &(Gnodo_archivo->tam_archivo), sizeof(uint32_t));

				tam_a_enviar += sizeof(int) + 2* sizeof(uint64_t) + sizeof(uint32_t);

			break;



		case ELIMINAR_ARCHIVO:

			pthread_mutex_lock(&m_truncar);

			log_info (loggerINFO, "El cliente quiere eliminar un archivo\n");

			estado = borrar_archivo(mensajeCliente);

			pthread_mutex_unlock(&m_truncar);

			lo_que_quiere =  malloc(sizeof(int));
			memcpy(lo_que_quiere, &estado, sizeof(int));

			tam_a_enviar = sizeof(int);

			break;

		case ELIMINAR_DIRECTORIO:

			pthread_mutex_lock(&m_truncar);

			log_info (loggerINFO, "El cliente quiere eliminar un directorio\n");

			estado = borrar_directorio(mensajeCliente);

			pthread_mutex_unlock(&m_truncar);

			lo_que_quiere =  malloc(sizeof(int));
			memcpy(lo_que_quiere, &estado, sizeof(int));

			tam_a_enviar = sizeof(int);
			break;

		case LISTAR_DIRECTORIO:

			log_info(loggerINFO,
					"El cliente quiere listar el directorio \"%s\"\n",
					mensajeCliente);
			t_list* lista_archivos = listar_archivos_del_directorio(mensajeCliente);

			if (list_is_empty(lista_archivos)) {

				estado = -3;
				lo_que_quiere = malloc (sizeof (int));
				memcpy(lo_que_quiere, &estado, sizeof(int));
				tam_a_enviar = sizeof (int);

				break;
			}
			//paso nombre + tipo + nro nodo
			Gfile* dato_archivo;
			lo_que_quiere = malloc(sizeof(int));
			int nro_nodo;
			uint8_t tipo;
			int tam_nombre;
			int tam_datos_archivo = 2*sizeof(int);
			int cant_archivos = list_size(lista_archivos);


			memcpy(lo_que_quiere, &tam_datos_archivo, sizeof(int));
			memcpy(lo_que_quiere + sizeof(int), &cant_archivos, sizeof(int));

			for (int i = 0; i < list_size(lista_archivos); i++) {

				dato_archivo = (Gfile*) list_get(lista_archivos, i);

				lo_que_quiere = realloc(lo_que_quiere, tam_datos_archivo + 2*sizeof(int) + strlen(dato_archivo->nombre_archivo) + sizeof(uint8_t));

				nro_nodo = indice_de_nodo(dato_archivo);
				tipo = dato_archivo->estado;
				tam_nombre = strlen(dato_archivo->nombre_archivo);

				memcpy(lo_que_quiere + tam_datos_archivo, &tam_nombre, sizeof(int));
				memcpy(lo_que_quiere + sizeof(int) +  tam_datos_archivo, dato_archivo->nombre_archivo, tam_nombre);
				memcpy(lo_que_quiere + sizeof(int) + tam_nombre + tam_datos_archivo, &tipo, sizeof(uint8_t));
				memcpy(lo_que_quiere + sizeof(int) + tam_nombre + sizeof(uint8_t) + tam_datos_archivo, &nro_nodo, sizeof(int));

				tam_datos_archivo += 2*sizeof(int) + strlen(dato_archivo->nombre_archivo) + sizeof(uint8_t);
			}

			tam_a_enviar = tam_datos_archivo;
			memcpy(lo_que_quiere, &tam_datos_archivo, sizeof(int));

			break;

		case ESCRIBIR_ARCHIVO:

			pthread_mutex_lock(&m_truncar);


			log_info(loggerINFO, "El cliente quiere escribir un archivo");


			lo_que_quiere = malloc (2 * sizeof(int));

			memcpy(&tam_path, mensajeCliente, sizeof(int));

			path = malloc(tam_path + 1);
			memcpy(path, mensajeCliente + sizeof(int), tam_path);

			memcpy(&tam_buf, mensajeCliente + sizeof(int) + tam_path, sizeof(int));
			buf = malloc(tam_buf + 1);

			memcpy(buf, mensajeCliente + tam_path + 2*sizeof(int), tam_buf);
			memcpy(&offset, mensajeCliente + tam_path + tam_buf + 2*sizeof(int), sizeof(int));

			path[tam_path] = '\0';
			buf[tam_buf] = '\0';

			int bytes_escritos = escribir_archivo(path, buf, tam_buf, offset);

			pthread_mutex_unlock(&m_truncar);

			if(bytes_escritos == -1) {
				estado = 1;
				memcpy(lo_que_quiere, &estado, sizeof(int));
				tam_a_enviar = sizeof(int);
				break;
			}


			else estado = 0;
			memcpy(lo_que_quiere, &estado, sizeof(int));
			memcpy(lo_que_quiere + sizeof(int), &bytes_escritos, sizeof(int));

			tam_a_enviar = 2*sizeof(int);

			free(buf);
			free(path);

			break;

		case CREAR_NODO:

			log_info(loggerINFO, "El cliente quiere crear un archivo.");
			lo_que_quiere = malloc(sizeof(int));

			memcpy (&tipo_nodo, mensajeCliente, sizeof (int));

			memcpy(&tam_path, mensajeCliente + sizeof (int), sizeof(int));

			path = malloc(tam_path + 1);

			memcpy(path, mensajeCliente + 2 * sizeof(int), tam_path);

			path[tam_path] = '\0';

			if (tipo_nodo == 2) {
				nodo = crear_elemento(OCUPADO, path, tabla_nodos);
			}

			else if (tipo_nodo == 3) {
				nodo = crear_elemento(DIRECTORIO, path, tabla_nodos);
			}

			if (nodo == -1) {
				estado = 1;
			} else estado = 0;

			tam_a_enviar = sizeof (int);

			free (path);

			memcpy(lo_que_quiere, &estado, sizeof(int));
			break;

		case ABRIR_DIRECTORIO:

			log_info (loggerINFO, "El cliente quiere abrir un directorio\n");
			directorio = buscar_archivo(mensajeCliente, tabla_nodos);

			if (directorio->estado != DIRECTORIO || directorio == NULL) {
				lo_que_quiere =  malloc(sizeof(int));
				estado = 1;
				memcpy(lo_que_quiere, &estado, sizeof(int));
				tam_a_enviar = sizeof(int);
				break;
			}
			else

		case ABRIR_ARCHIVO:

			log_info(loggerINFO, "El cliente quiere abrir un archivo con el path %s.\n", mensajeCliente);


			int nodo_archivo;

			if ((nodo_archivo = buscar_indice_nodo_por_path(mensajeCliente)) == -1) {
				lo_que_quiere =  malloc(sizeof(int));
				estado = 1;
				memcpy(lo_que_quiere, &estado, sizeof(int));
				tam_a_enviar = sizeof(int);
				break;
			}
			else estado = 0;
			lo_que_quiere =  malloc(2*sizeof(int));
			memcpy(lo_que_quiere, &estado, sizeof(int));
			memcpy(lo_que_quiere + sizeof(int), &nodo_archivo, sizeof(int));
			tam_a_enviar = 2*sizeof(int);

			break;

		case TRUNCAR:

			pthread_mutex_lock(&m_truncar);

			log_info (loggerINFO, "Estoy truncando...........");

			memcpy (&offset, mensajeCliente, sizeof (int));
			memcpy (&tam_path, mensajeCliente + sizeof (int), sizeof (int));
			path = malloc (tam_path + 1);
			memcpy (path, mensajeCliente + 2*sizeof (int), tam_path);
			path [tam_path] = '\0';


			lo_que_quiere = malloc (sizeof (int));

			tam_a_enviar = sizeof (int);

			estado = truncar_archivo (path, offset);
			free (path);

			memcpy (lo_que_quiere, &estado, sizeof (int));

			pthread_mutex_unlock(&m_truncar);
			break;
		}
		int mensajito;
		pthread_mutex_lock (&sockete);
		if (codigo_operacion == 0) {
			int error = -1;
			if (send(datos_cliente.new_fd, &error, sizeof(int), 0) == -1) {
						perror("send\n");
						free (mensajeCliente);
						free (lo_que_quiere);
						recv (datos_cliente.new_fd, &mensajito, sizeof (int), 0);
						pthread_mutex_unlock (&sockete);
						pthread_exit(NULL);
					}
			free (lo_que_quiere);
		}
		//-------------  Le respondo al cliente

		else {


		//sacar a la mierda
			int bytes_enviados;
		if ((bytes_enviados =  send(datos_cliente.new_fd, lo_que_quiere, tam_a_enviar, 0)) == -1) {
			perror("send\n");
			free (mensajeCliente);
			free (lo_que_quiere);
			recv (datos_cliente.new_fd, &mensajito, sizeof (int), 0);
			pthread_mutex_unlock (&sockete);
			pthread_exit(NULL);
		}



		printf("bytes enviados en send desde el servidor: %i\n", bytes_enviados);


		free(lo_que_quiere);
		printf("\n~~~~~~~~~~~~~~~~~~~~~~~\n");
		}

		recv (datos_cliente.new_fd, &mensajito, sizeof (int), 0);
		pthread_mutex_unlock (&sockete);
	//}

	close(datos_cliente.new_fd);
	free(mensajeCliente);
	pthread_mutex_unlock (&thread);
}

