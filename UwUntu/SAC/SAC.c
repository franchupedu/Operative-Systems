
#include "escuchar-pedidos.h"



int main(int argc, char* argv[]) {

/*
		if ( argc > 1 && strcmp(argv[1], "--new_disk") == 0) {
			crear_disco();
			formatear();
			printf("if 2\n");
		}*/

	pthread_mutex_init(&m_truncar, NULL);
	pthread_mutex_init(&thread, NULL);
	pthread_mutex_init(&sockete, NULL);

	int posicion_header = 0;
	int posicion_bitmap = 1;
	int posicion_tabla_nodos;

	iniciar_logs_disco();


	crear_disco();
	formatear();

	int discofd;
	if ((discofd = open(nombre_disco, O_RDWR, 0)) == -1) {
		log_info(loggerERROR_DISK, "error\n");
		exit(1);
	}

	int tamanio_disco = largo_archivo(nombre_disco);
	log_info(loggerINFO_DISK, "Tamaño del disco en bytes: %i\n", tamanio_disco);

	disco = mmap(NULL, tamanio_disco, PROT_READ | PROT_WRITE,
			MAP_FILE | MAP_SHARED, discofd, 0);
	if (disco == MAP_FAILED) {
		close(discofd);
		perror("mmap");
		exit(-1);
	}
	close(discofd);


	cabecera = (Header*) (disco + posicion_header);

	posicion_tabla_nodos = posicion_bitmap + cabecera->bloques_bitmap;

	comienzo_bloques_datos = posicion_tabla_nodos + MAX_FILE_COUNT;
	cant_bloques_datos = tamanio_disco / BLOCK_SIZE - 1
			- cabecera->bloques_bitmap - MAX_FILE_COUNT;
	cant_bloques_totales = tamanio_disco / BLOCK_SIZE;

	bitmap_en_disco = (char*) (disco + posicion_bitmap);
	tabla_nodos = (Gfile*) (disco + posicion_tabla_nodos);

	size_t tamanio_bitmap_bytes = BLOCK_SIZE * cabecera->bloques_bitmap;
	log_info(loggerINFO_DISK, "tamanio_bitmap_bytes: %i\n", tamanio_bitmap_bytes);

	bitmap = bitarray_create_with_mode(bitmap_en_disco, tamanio_bitmap_bytes,
			LSB_FIRST);  //Ver si es correcto

	msync(disco, tamanio_disco, MS_SYNC);

	crear_raiz();
	datos_server();
	iniciar_logs();

	int sockfd, new_fd; // Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
	struct sockaddr_in my_addr;    // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del cliente
	int sin_size;
	int yes = 1;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	my_addr.sin_family = AF_INET;         // Ordenación de bytes de la máquina
	my_addr.sin_port = htons(puerto);    // short, Ordenación de bytes de la red
	my_addr.sin_addr.s_addr = inet_addr(ip); // Rellenar con mi dirección IP
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura

	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))
			== -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sockfd, SOMAXCONN) == -1) {
		perror("listen");
		exit(1);
	}

	printf("Estoy escuchando, invita a tus amigos al server %s en el "
			"puerto %d\n", inet_ntoa(my_addr.sin_addr), puerto);

	int cont = 0;
	while (1) {  // main accept() loop
		sin_size = sizeof(struct sockaddr_in);

		pthread_mutex_lock (&thread);

		if ((new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size))
				== -1) {
			perror("accept");
			continue;
		}
		log_info(loggerINFO, "\nserver: got connection from %s in %i\n",
				inet_ntoa(their_addr.sin_addr), new_fd);

		struct cliente_op nuevo_cliente;
		nuevo_cliente.new_fd = new_fd;
		nuevo_cliente.their_addr = their_addr;

		pthread_t thread_escuchar;
		pthread_attr_t atributos;
		pthread_attr_init(&atributos);
		pthread_attr_setdetachstate(&atributos, PTHREAD_CREATE_DETACHED);
		// Acá creo el hilo
		int estado = pthread_create(&thread_escuchar, &atributos,
		atender_pedidos, &nuevo_cliente);
		pthread_detach (thread_escuchar);
		log_info(loggerINFO, "Estado del hilo: %i\n", estado);

		cont++;
	}

	log_info(loggerERROR, "me salí del while\n");
	return 0;

}



