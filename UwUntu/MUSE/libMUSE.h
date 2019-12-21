#ifndef LIBMUSE_H_
#define LIBMUSE_H_

    #include <stdint.h>
    #include <stddef.h>
	#include <pthread.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <errno.h>
	#include <string.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <sys/wait.h>
	#include <signal.h>
	#include <commons/process.h>

	int sd;
	struct sockaddr_in add;

	    void mandarInt(int sd, int valor)
	    {
	    	int* mensaje = malloc(sizeof(int));
	    	*mensaje = valor;
	    	send(sd, mensaje, sizeof(int), 0);
	    	free(mensaje);

	    }

	    int recibirInt(int sd)
	    {
	    	int* mensaje = malloc(sizeof(int));
	    	int resultado;
	    	resultado = recv(sd, mensaje, sizeof(int), 0);
	    	int retorno = *mensaje;
	    	free(mensaje);
	    	if(resultado == 0)
	    	{
	    		return -1;
	    	}
	    	return retorno;
	    }

	    void mandarVoid(int sd, void* valor, int n)
	    {
	    	mandarInt(sd, n);
	    	send(sd, valor, n, 0);
	    }

	    void* recibirVoid(int sd)
	    {
	    	int longitud = recibirInt(sd);
	    	void* mensaje = malloc(longitud + 100);
	    	recv(sd, mensaje, longitud, 0);
	    	return mensaje;
	    }

    /**
     * Inicializa la biblioteca de MUSE.
     * @param id El Process (o Thread) ID para identificar el caller en MUSE.
     * @param ip IP en dot-notation de MUSE.
     * @param puerto Puerto de conexión a MUSE.
     * @return Si pasa un error, retorna -1. Si se inicializó correctamente, retorna 0.
     * @see Para obtener el id de un proceso pueden usar getpid() de la lib POSIX (unistd.h)
     * @note Debido a la naturaleza centralizada de MUSE, esta función deberá definir
     *  el ID del proceso/hilo según "IP-ID".
     */
    int muse_init(int id, char* ip, int puerto)
    {
    	sd = socket(AF_INET, SOCK_STREAM, 0);

    	add.sin_family = AF_INET;
        add.sin_addr.s_addr = inet_addr(ip);
        add.sin_port = htons(puerto);

    	connect(sd, (struct sockaddr *) &add, sizeof(add));

    	mandarInt(sd, 1);

      	mandarInt(sd, process_getpid());

    	int respuesta = recibirInt(sd);

    	if(respuesta == 0)
    	{
    		return 0;
    	}

    	return -1;
    }

    /**
     * Cierra la biblioteca de MUSE.
     */
    void muse_close()
    {
    	mandarInt(sd, 2);

    	mandarInt(sd, process_getpid());
    }

    /**
     * Reserva una porción de memoria contígua de tamaño `tam`.
     * @param tam La cantidad de bytes a reservar.
     * @return La dirección de la memoria reservada.
     */
    uint32_t muse_alloc(uint32_t tam)
    {
    	if(tam == 0)
    		return 0;

    	mandarInt(sd, 3);

    	mandarInt(sd, process_getpid());

    	mandarInt(sd, tam);

    	return recibirInt(sd);
    }

    /**
     * Libera una porción de memoria reservada.
     * @param dir La dirección de la memoria a reservar.
     */
    void muse_free(uint32_t dir)
    {
    	mandarInt(sd, 4);

		mandarInt(sd, process_getpid());

		mandarInt(sd, dir);
    }

    /**
     * Copia una cantidad `n` de bytes desde una posición de memoria de MUSE a una `dst` local.
     * @param dst Posición de memoria local con tamaño suficiente para almacenar `n` bytes.
     * @param src Posición de memoria de MUSE de donde leer los `n` bytes.
     * @param n Cantidad de bytes a copiar.
     * @return Si pasa un error, retorna -1. Si la operación se realizó correctamente, retorna 0.
     */
    int muse_get(void* dst, uint32_t src, size_t n)
    {
    	if(n == 0)
    		return 0;

    	mandarInt(sd, 5);

		mandarInt(sd, process_getpid());

		mandarVoid(sd, dst, n);

		mandarInt(sd, src);

		mandarInt(sd, n);

		int res = recibirInt(sd);

		if(res == -1)
		{
			printf("So you have chosen death\n");
			kill(0, 11);
			return -1;
		}

		void* resp = recibirVoid(sd);

		memcpy(dst, resp, n);

		return 0;
    }

    /**
     * Copia una cantidad `n` de bytes desde una posición de memoria local a una `dst` en MUSE.
     * @param dst Posición de memoria de MUSE con tamaño suficiente para almacenar `n` bytes.
     * @param src Posición de memoria local de donde leer los `n` bytes.
     * @param n Cantidad de bytes a copiar.
     * @return Si pasa un error, retorna -1. Si la operación se realizó correctamente, retorna 0.
     */
    int muse_cpy(uint32_t dst, void* src, int n)
    {
    	if(n == 0)
    		return 0;

    	mandarInt(sd, 6);

    	mandarInt(sd, process_getpid());

    	mandarVoid(sd, src, n);

    	mandarInt(sd, dst);

    	mandarInt(sd, n);

    	int ret = recibirInt(sd);

    	if(ret == -1)
    	{
    		printf("So you have chosen death\n");
    		kill(0, 11);
    		return -1;
    	}
    	return ret;
    }


    /**
     * Devuelve un puntero a una posición mappeada de páginas por una cantidad `length` de bytes el archivo del `path` dado.
     * @param path Path a un archivo en el FileSystem de MUSE a mappear.
     * @param length Cantidad de bytes de memoria a usar para mappear el archivo.
     * @param flags
     *          MAP_PRIVATE     Solo un proceso/hilo puede mappear el archivo.
     *          MAP_SHARED      El segmento asociado al archivo es compartido.
     * @return Retorna la posición de memoria de MUSE mappeada.
     * @note: Si `length` sobrepasa el tamaño del archivo, toda extensión deberá estar llena de "\0".
     * @note: muse_free no libera la memoria mappeada. @see muse_unmap
     */
    uint32_t muse_map(char *path, size_t length, int flags)
    {
    	if(length == 0)
    		return 0;

    	mandarInt(sd, 7);

    	mandarInt(sd, process_getpid());

    	mandarVoid(sd, path, strlen(path)+1);

    	mandarInt(sd, length);

    	mandarInt(sd, flags);

    	return recibirInt(sd);
    }

    /**
     * Descarga una cantidad `len` de bytes y lo escribe en el archivo en el FileSystem.
     * @param addr Dirección a memoria mappeada.
     * @param len Cantidad de bytes a escribir.
     * @return Si pasa un error, retorna -1. Si la operación se realizó correctamente, retorna 0.
     * @note Si `len` es menor que el tamaño de la página en la que se encuentre, se deberá escribir la página completa.
     */
    int muse_sync(uint32_t addr, size_t len)
    {
    	if(len == 0)
    		return 0;

    	mandarInt(sd, 8);

    	mandarInt(sd, process_getpid());

    	mandarInt(sd, addr);

    	mandarInt(sd, len);

    	return recibirInt(sd);
    }

    /**
     * Borra el mappeo a un archivo hecho por muse_map.
     * @param dir Dirección a memoria mappeada.
     * @param
     * @note Esto implicará que todas las futuras utilizaciones de direcciones basadas en `dir` serán accesos inválidos.
     * @note Solo se deberá cerrar el archivo mappeado una vez que todos los hilos hayan liberado la misma cantidad de muse_unmap que muse_map.
     * @return Si pasa un error, retorna -1. Si la operación se realizó correctamente, retorna 0.
     */
    int muse_unmap(uint32_t dir)
    {
    	mandarInt(sd, 9);

    	mandarInt(sd, process_getpid());

    	mandarInt(sd, dir);

    	return recibirInt(sd);
    }


#endif
