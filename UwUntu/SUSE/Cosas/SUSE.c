#include "SUSE.h"

void metricas () {
	struct finalizado *recorrido = finalizados;
	struct programa *programa;
	struct semaforo *semaforo;
	struct finalizado *aux;
	struct ult *ult;
	int tiempo;

	pthread_mutex_lock (&finalizadoses);

	while (recorrido) {
		pthread_mutex_lock (&logs);

		log_info (log, "El tiempo de ejecución del hilo %i del programa %i fue %i.", recorrido -> tid, recorrido -> pid, recorrido -> ejecucion);

		pthread_mutex_unlock (&logs);

		aux = recorrido;

		recorrido = recorrido -> sig;

		free (aux);
	}

	finalizados = NULL;

	pthread_mutex_unlock (&finalizadoses);

	pthread_mutex_lock (&programases);

	programa = programas;

	while (programa) {
		programa -> listos = 0;

		programa -> ejecutando = 0;

		programa -> bloqueados = 0;

		ult = programa -> ults;

		tiempo = 0;

		while (ult) {
			tiempo += ult -> ejecucion;

			ult = ult -> sig;
		}

		ult = programa -> ults;

		if (tiempo) {
			while (ult) {
				pthread_mutex_lock (&logs);

				log_info (log, "El tiempo de espera del hilo %i del programa %i es %i.", ult -> tid, programa -> pid, ult -> espera);

				log_info (log, "El tiempo de ejecución del hilo %i del programa %i es %i.", ult -> tid, programa -> pid, ult -> ejecucion);

				log_info (log, "El porcentaje de tiempo de ejecución del hilo %i del programa %i es %i.", ult -> tid, programa -> pid, 100 * ult -> ejecucion / tiempo);

				pthread_mutex_unlock (&logs);

				switch (ult -> estado) {
					case Listo: programa -> listos++; break;
					case Ejecutando: programa -> ejecutando++; break;
					case Bloqueado: programa -> bloqueados++; break;
				}

				ult = ult -> sig;
			}

			pthread_mutex_lock (&logs);

			log_info (log, "La cantidad de hilos nuevos en el programa %i es de 0, la de hilos listos es de %i, la de hilos en ejecución es de %i y la de bloqueados %i.", programa -> pid, programa -> listos, programa -> ejecutando, programa -> bloqueados);

			pthread_mutex_unlock (&logs);
		}

		programa = programa -> sig;
	}

	pthread_mutex_unlock (&programases);

	pthread_mutex_lock (&semaforoses);

	semaforo = semaforos;

	while (semaforo) {
		pthread_mutex_lock (&logs);

		log_info (log, "El valor actual del semáforo %s es de %i.", semaforo -> sid, semaforo -> val);

		pthread_mutex_unlock (&logs);

		semaforo = semaforo -> sig;
	}

	pthread_mutex_unlock (&semaforoses);

	pthread_mutex_lock (&logs);

	log_info (log, "El grado actual de multiprogramación es de %i.", multi);

	pthread_mutex_unlock (&logs);
}

void *ciclo () {
	sleep (config_get_int_value (config, "Timer"));

	metricas ();

	ciclo ();

	return NULL;
}

void suse_init () {
	char ***vals = config_get_array_value (config, "Vals");
	char ***maxs = config_get_array_value (config, "Maxs");
	char ***sids = config_get_array_value (config, "Sids");
	int cant = config_get_int_value (config, "Semáforos");

	pthread_mutex_lock (&semaforoses);

	pthread_mutex_lock (&logs);

	log_info (log, "Inicializando semáforos.");

	pthread_mutex_unlock (&logs);

	for (int i = 0; i < cant; i++) {
		struct semaforo *semaforo = malloc (sizeof (struct semaforo));

		semaforo -> sig = semaforos;

		semaforo -> sid = strdup (sids [i]);

		semaforo -> max = atoi (maxs [i]);

		semaforo -> val = atoi (vals [i]);

		semaforo -> semaforeados = NULL;

		semaforos = semaforo;
	}

	pthread_mutex_lock (&logs);

	log_info (log, "Semáforos iniciados con éxito.");

	pthread_mutex_unlock (&logs);

	pthread_mutex_unlock (&semaforoses);
}

struct programa *buscarprograma (int pid) {
	struct programa *recorrido;

	pthread_mutex_lock (&programases);

	recorrido = programas;

	while (recorrido) {
		if (recorrido -> pid == pid) {
			pthread_mutex_unlock (&programases);

			return recorrido;
		}

		recorrido = recorrido -> sig;
	}

	pthread_mutex_unlock (&programases);

	return NULL;
}

void crear (struct programa *programa, char *request) {
	struct ult *ult;

	pthread_mutex_lock (programa -> mutex);

	ult = malloc (sizeof (struct ult));

	pthread_mutex_lock (&logs);

	log_info (log, "Creando un ult para el programa %i.", programa -> pid);

	pthread_mutex_unlock (&logs);

	ult -> tid = (request [1] - 48) * 1000 + (request [2] - 48) * 100 + (request [3] - 48) * 10 + request [4] - 48;

	ult -> creacion = (int) clock ();

	ult -> empieza = (int) clock ();

	ult -> ejecucion = 0;

	ult -> estimacion = 0;

	ult -> duracion = 0;

	ult -> estado = Listo;

	ult -> espera = 0;

	ult -> join = 0;

	ult -> sig = programa -> ults;

	programa -> ults = ult;

	pthread_mutex_lock (&logs);

	log_info (log, "El ult será el %i.", ult -> tid);

	pthread_mutex_unlock (&logs);

	if (! programa -> tienehilos) {
		pthread_mutex_unlock (programa -> hilos);

		programa -> tienehilos = 1;
	}

	pthread_mutex_unlock (programa -> mutex);
}

double estimacion (double estimacion, int duracion) {
	return estimacion * config_get_double_value (config, "Alfa") + duracion * (1 - config_get_double_value (config, "Alfa"));
}

void planificar (struct programa *programa) {
	struct ult *recorrido;
	double estimado = -1;
	char *tidcasteado;
	double mejor;
	int tid;

	if (gradodemultiprogramacion - 1 < 0)
		pthread_mutex_lock (&multiprogramacion);

	gradodemultiprogramacion--;

	pthread_mutex_lock (&multio);

	multi++;

	pthread_mutex_unlock (&multio);

	programa -> planificando = 1;

	if (! programa -> tienehilos)
		pthread_mutex_lock (programa -> hilos);

	if (programa -> u1) {
		programa -> u1 = 0;

		pthread_mutex_lock (programa -> join);
	}

	pthread_mutex_lock (programa -> mutex);

	recorrido = programa -> ults;

	tidcasteado = malloc (5);

	pthread_mutex_lock (&logs);

	log_info (log, "Planificando el siguente ult del programa %i.", programa -> pid);

	pthread_mutex_unlock (&logs);

	while (recorrido) {
		if (recorrido -> estado == Ejecutando) {
			recorrido -> estado = Listo;

			recorrido -> duracion = (int) clock () - recorrido -> inicio;

			recorrido -> empieza = (int) clock ();

			recorrido -> ejecucion += recorrido -> duracion;

			break;
		}

		recorrido = recorrido -> sig;
	}

	if (recorrido) {
		if (gradodemultiprogramacion + 1 < config_get_int_value (config, "Multiprogramación")) {
				gradodemultiprogramacion++;

				pthread_mutex_unlock (&multiprogramacion);
		}

		pthread_mutex_lock (&multio);

		multi--;

		pthread_mutex_unlock (&multio);
	}

	recorrido = programa -> ults;

	while (recorrido) {
		if (recorrido -> estado == Listo) {
			if (! (programa -> joineado == Nuevo && ! recorrido -> tid)) {
				estimado = estimacion (recorrido -> estimacion, recorrido -> duracion);

				mejor = estimado;

				tid = recorrido -> tid;

				break;
			}
		}

		recorrido = recorrido -> sig;

		if (! recorrido) {
			if (estimado < 0) {
				programa -> bloqueado = 1;

				if (gradodemultiprogramacion + 1 < config_get_int_value (config, "Multiprogramación")) {
						gradodemultiprogramacion++;

						pthread_mutex_unlock (&multiprogramacion);
				}

				pthread_mutex_lock (&multio);

				multi--;

				pthread_mutex_unlock (&multio);

				pthread_mutex_unlock (programa -> mutex);

				pthread_mutex_lock (programa -> bloqueadoes);

				if (gradodemultiprogramacion - 1 < 0)
					pthread_mutex_lock (&multiprogramacion);

				gradodemultiprogramacion--;

				pthread_mutex_lock (&multio);

				multi++;

				pthread_mutex_unlock (&multio);

				pthread_mutex_lock (programa -> mutex);
			}

			recorrido = programa -> ults;
		}
	}

	recorrido = programa -> ults;

	while (recorrido) {
		if (recorrido -> estado == Listo) {
			if (! (programa -> joineado == Nuevo && ! recorrido -> tid)) {
				estimado = estimacion (recorrido -> estimacion, recorrido -> duracion);

				pthread_mutex_lock (&logs);

				log_info (log, "El estimado del ult %i es %f.", recorrido -> tid, estimado);

				pthread_mutex_unlock (&logs);

				if (estimado < mejor) {
					mejor = estimado;

					tid = recorrido -> tid;
				}
			}
		}

		recorrido = recorrido -> sig;
	}

	pthread_mutex_lock (&logs);

	log_info (log, "El que sigue es el ult %i con el estimado %f.", tid, mejor);

	pthread_mutex_unlock (&logs);

	recorrido = programa -> ults;

	while (recorrido) {
		if (recorrido -> tid == tid)
			break;

		recorrido = recorrido -> sig;

		if (! recorrido)
			recorrido = programa -> ults;
	}

	tidcasteado [0] = tid / 1000 + 48;

	tidcasteado [1] = tid / 100 - (tid / 1000) * 10 + 48;

	tidcasteado [2] = tid / 10 - (tid / 100) * 10 + 48;

	tidcasteado [3] = tid - (tid / 10) * 10 + 48;

	tidcasteado [4] = '\0';

	recorrido -> estimacion = mejor;

	recorrido -> inicio = (int) clock ();

	recorrido -> estado = Ejecutando;

	recorrido -> espera += (int) clock () - recorrido -> empieza;

	send (programa -> pid, tidcasteado, 5, 0);

	free (tidcasteado);

	if (! programa -> u0 && ! tid) {
		programa -> u0 = 1;

		programa -> u1 = 1;

		pthread_mutex_unlock (programa -> ult0);
	}

	programa -> planificando = 0;

	pthread_mutex_unlock (programa -> mutex);
}

void joinear (struct programa *programa, char *request) {
	struct ult *recorrido;
	int tid;

	if (! programa -> u0)
		pthread_mutex_lock (programa -> join);

	if (! programa -> u0)
		pthread_mutex_lock (programa -> ult0);

	pthread_mutex_lock (programa -> mutex);

	recorrido = programa -> ults;

	while (recorrido) {
		if (! recorrido -> tid && recorrido -> estado != Bloqueado) {
			if (recorrido -> estado == Ejecutando) {
				if (gradodemultiprogramacion + 1 < config_get_int_value (config, "Multiprogramación")) {
						gradodemultiprogramacion++;

						pthread_mutex_unlock (&multiprogramacion);
				}

				pthread_mutex_lock (&multio);

				multi--;

				pthread_mutex_unlock (&multio);

			}

			recorrido -> estado = Bloqueado;

			recorrido -> duracion = (int) clock () - recorrido -> inicio;

			recorrido -> espera += (int) clock () - recorrido -> empieza;

			recorrido -> ejecucion += recorrido -> duracion;

			break;
		}

		recorrido = recorrido -> sig;
	}

	tid = (request [1] - 48) * 1000 + (request [2] - 48) * 100 + (request [3] - 48) * 10 + request [4] - 48;

	pthread_mutex_lock (&logs);

	log_info (log, "Joineando al ult %i.", tid);

	pthread_mutex_unlock (&logs);

	recorrido = programa -> ults;

	while (recorrido) {
		if (recorrido -> tid == tid)
			break;

		recorrido = recorrido -> sig;
	}

	if (recorrido)
		recorrido -> join = 1;

	else {
		recorrido = programa -> ults;

		while (recorrido) {
			if (! recorrido -> tid) {
				recorrido -> estado = Listo;

				recorrido -> empieza = (int) clock ();

				break;
			}

			recorrido = recorrido -> sig;

			if (! recorrido)
				recorrido = programa -> ults;
		}
	}

	programa -> joineado = Joineado;

	if (programa -> u1)
		pthread_mutex_unlock (programa -> join);

	pthread_mutex_unlock (programa -> mutex);
}

void cerrar (struct programa *programa, char *request) {
	struct finalizado *finalizado;
	struct ult *recorrido;
	struct ult *ult;
	int hilos = 0;
	int ahora;
	int tid;

	if (programa) {
		pthread_mutex_lock (programa -> mutex);

		ult = programa -> ults;

		tid = (request [1] - 48) * 1000 + (request [2] - 48) * 100 + (request [3] - 48) * 10 + request [4] - 48;

		pthread_mutex_lock (&logs);

		log_info (log, "Cerrando el ult %i.", tid);

		pthread_mutex_unlock (&logs);

		while (ult) {
			if (ult -> tid == tid)
				break;

			ult = ult -> sig;

			if (! ult)
				ult = programa -> ults;
		}

		recorrido = programa -> ults;

		while (recorrido) {
			hilos++;

			recorrido = recorrido -> sig;
		}

		recorrido = programa -> ults;

		if (hilos == 2) {
			pthread_mutex_lock (&logs);

			log_info (log, "Liberando los ults joineados.");

			pthread_mutex_unlock (&logs);

			while (recorrido) {
				if (! recorrido -> tid) {
					programa -> joineado = Liberado;

					recorrido -> estado = Listo;

					recorrido -> empieza = (int) clock ();

					break;
				}

				recorrido = recorrido -> sig;

				if (! recorrido)
					recorrido = programa -> ults;
			}
		}

		recorrido = programa -> ults;

		if (recorrido -> tid == tid)
			programa -> ults = ult -> sig;

		else {
			while (recorrido -> sig -> tid != tid)
				recorrido = recorrido -> sig;

			recorrido -> sig = ult -> sig;
		}

		pthread_mutex_lock (&logs);

		log_info (log, "Liberando la memoria del ult %i.", ult -> tid);

		pthread_mutex_unlock (&logs);

		pthread_mutex_unlock (programa -> mutex);

		pthread_mutex_lock (&finalizadoses);

		ahora = (int) clock ();

		finalizado = malloc (sizeof (struct finalizado));

		finalizado -> pid = programa -> pid;

		finalizado -> tid = ult -> tid;

		finalizado -> ejecucion = ahora - ult -> creacion;

		finalizado -> sig = finalizados;

		finalizados = finalizado;

		pthread_mutex_unlock (&finalizadoses);

		pthread_mutex_unlock (programa -> mutex);

		if (ult -> estado == Ejecutando) {
			if (gradodemultiprogramacion + 1 < config_get_int_value (config, "Multiprogramación")) {
					gradodemultiprogramacion++;

					pthread_mutex_unlock (&multiprogramacion);
			}

			pthread_mutex_lock (&multio);

			multi--;

			pthread_mutex_unlock (&multio);
		}

		free (ult);

		metricas ();
	}
}

void wait (struct programa *programa, char *request) {
	struct semaforeado *semaforeado;
	struct semaforo *semaphore;
	struct ult *recorrido;
	int tid;

	pthread_mutex_lock (programa -> mutex);

	tid = (request [1] - 48) * 1000 + (request [2] - 48) * 100 + (request [3] - 48) * 10 + request [4] - 48;

	pthread_mutex_lock (&semaforoses);

	semaphore = semaforos;

	while (semaphore && strcmp (semaphore -> sid, request + 6))
		semaphore = semaphore -> sig;

	if (semaphore) {
		pthread_mutex_lock (&logs);

		log_info (log, "Intentando concender el semáforo %s al hilo %i.", semaphore -> sid, tid);

		pthread_mutex_unlock (&logs);

		recorrido = programa -> ults;

		while (recorrido && recorrido -> tid != tid)
			recorrido = recorrido -> sig;

		if (semaphore -> val - 1 < 0) {
			semaforeado = malloc (sizeof (struct semaforeado));

			semaforeado -> pid = programa -> pid;

			semaforeado -> tid = tid;

			semaforeado -> sig = semaphore -> semaforeados;

			semaphore -> semaforeados = semaforeado;

			if (recorrido -> estado == Ejecutando) {
				if (gradodemultiprogramacion + 1 < config_get_int_value (config, "Multiprogramación")) {
						gradodemultiprogramacion++;

						pthread_mutex_unlock (&multiprogramacion);
				}

				pthread_mutex_lock (&multio);

				multi--;

				pthread_mutex_unlock (&multio);
			}

			recorrido -> estado = Bloqueado;

			pthread_mutex_lock (&logs);

			log_info (log, "El hilo %i fue bloqueado por su pedido de semáforo %s.", tid, semaphore -> sid);

			pthread_mutex_unlock (&logs);
		}

		else {
			semaphore -> val--;

			pthread_mutex_lock (&logs);

			log_info (log, "El semáforo %s fue concedido satisfactoriamente al hilo %i.", semaphore -> sid, tid);

			pthread_mutex_unlock (&logs);
		}
	}

	pthread_mutex_unlock (&semaforoses);

	pthread_mutex_unlock (programa -> mutex);
}

void signal (char *request) {
	struct semaforeado *dessemaforeado;
	struct semaforeado *auxiliar;
	struct programa *recorrido;
	struct programa *programa;
	struct semaforo *semaforo;
	struct ult *aux;

	pthread_mutex_lock (&semaforoses);

	pthread_mutex_lock (&programases);

	semaforo = semaforos;

	while (semaforo && strcmp (semaforo -> sid, request + 2))
		semaforo = semaforo -> sig;

	if (semaforo) {
		pthread_mutex_lock (&logs);

		log_info (log, "Signaleando el semáforo %s.", semaforo -> sid);

		pthread_mutex_unlock (&logs);

		dessemaforeado = semaforo -> semaforeados;

		while (dessemaforeado && dessemaforeado -> sig)
			dessemaforeado = dessemaforeado -> sig;

		auxiliar = semaforo -> semaforeados;

		if (auxiliar) {
			if (auxiliar -> sig) {
				while (auxiliar -> sig -> sig)
					auxiliar = auxiliar -> sig;

				auxiliar -> sig = NULL;
			}

			else
				semaforo -> semaforeados = NULL;
		}

		recorrido = programas;

		if (dessemaforeado) {
			while (recorrido) {
				if (recorrido -> pid != dessemaforeado -> pid) {
					recorrido = recorrido -> sig;

					continue;
				}

				aux = recorrido -> ults;

				while (aux) {
					if (aux -> tid == dessemaforeado -> tid) {
						aux -> estado = Listo;

						if (recorrido -> bloqueado) {
							recorrido -> bloqueado = 0;

							pthread_mutex_unlock (recorrido -> bloqueadoes);
						}

						pthread_mutex_lock (&logs);

						log_info (log, "Desbloqueando el hilo %i del proceso %i.", dessemaforeado -> tid, dessemaforeado -> pid);

						pthread_mutex_unlock (&logs);

						free (dessemaforeado);

						break;
					}

					aux = aux -> sig;
				}

				break;
			}
		}

		else {
			if (semaforo -> val + 1 <= semaforo -> max)
				semaforo -> val++;
		}
	}

	pthread_mutex_unlock (&programases);

	pthread_mutex_unlock (&semaforoses);
}

void *atender (void *argumentos) {
	struct parametros *parametros = argumentos;

	if (buscarprograma (parametros -> cliente)) {
		pthread_mutex_lock (&logs);

		log_info (log, "Atendiendo al cliente %i.", parametros -> cliente);

		pthread_mutex_unlock (&logs);

		switch (parametros -> request [0]) {
			case 1: crear (buscarprograma (parametros -> cliente), parametros -> request); break;
			case 2: planificar (buscarprograma (parametros -> cliente)); break;
			case 3: joinear (buscarprograma (parametros -> cliente), parametros -> request); break;
			case 4: cerrar (buscarprograma (parametros -> cliente), parametros -> request); break;
			case 5: wait (buscarprograma (parametros -> cliente), parametros -> request); break;
			case 6: signal (parametros -> request); break;
		}
	}

	free (parametros -> request);

	free (parametros);

	return NULL;
}

void *conectar (void *cliente) {
	struct programa *programa;
	int nuevofd;

	memcpy (&nuevofd, cliente, sizeof (int));

	pthread_mutex_unlock (&mcliente);

	pthread_mutex_lock (&logs);

	log_info (log, "Nueva conexión en %i.", nuevofd);

	pthread_mutex_unlock (&logs);

	programa = malloc (sizeof (struct programa));

	programa -> pid = nuevofd;

	programa -> ults = NULL;

	programa -> joineado = Nuevo;

	programa -> planificando = 0;

	programa -> tienehilos = 0;

	programa -> bloqueado = 0;

	programa -> u0 = 0;

	programa -> u1 = 0;

	programa -> bloqueadoes = malloc (sizeof (pthread_mutex_t));

	pthread_mutex_init (programa -> bloqueadoes, NULL);

	pthread_mutex_lock (programa -> bloqueadoes);

	programa -> join = malloc (sizeof (pthread_mutex_t));

	pthread_mutex_init (programa -> join, NULL);

	programa -> ult0 = malloc (sizeof (pthread_mutex_t));

	pthread_mutex_init (programa -> ult0, NULL);

	pthread_mutex_lock (programa -> ult0);

	programa -> mutex = malloc (sizeof (pthread_mutex_t));

	pthread_mutex_init (programa -> mutex, NULL);

	programa -> hilos = malloc (sizeof (pthread_mutex_t));

	pthread_mutex_init (programa -> hilos, NULL);

	pthread_mutex_lock (programa -> hilos);

	pthread_mutex_lock (&programases);

	programa -> sig = programas;

	programas = programa;

	pthread_mutex_unlock (&programases);

	pthread_mutex_unlock (&conectando);

	return NULL;
}

void despertar () {
	struct sockaddr_in direccioncliente;
	struct parametros *parametros;
	char *mensaje = malloc (1);
	struct programa *recorrido;
	struct programa *programa;
	int tamaniomensaje;
	struct ult *ult;
	struct ult *aux;
	fd_set temporal;
	fd_set general;
	int maximofd;
	int nuevofd;

	FD_ZERO (&general);

	FD_ZERO (&temporal);

	struct sockaddr_in direccionservidor;
	direccionservidor.sin_family = AF_INET;
	direccionservidor.sin_addr.s_addr = inet_addr (config_get_string_value (config, "IP"));
	direccionservidor.sin_port = htons (config_get_int_value (config, "Puerto"));

	pthread_mutex_lock (&logs);

	log_info (log, "Levantando servidor.");

	pthread_mutex_unlock (&logs);

	int servidor = socket (AF_INET, SOCK_STREAM, 0);

	int activado = 1;

	pthread_mutex_lock (&logs);

	log_info (log, "Servidor levantado en el fichero %i.", servidor);

	pthread_mutex_unlock (&logs);

	setsockopt (servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof (activado));

	if (bind (servidor, (void*) &direccionservidor, sizeof (direccionservidor)) != 0) {
		perror ("Falló el bind.\n");

		log_info (log, "\"Address already in use\" re cualquiera je :}.");

		exit (EXIT_FAILURE);
	}

	pthread_mutex_lock (&logs);

	log_info (log, "Estoy escuchando.");

	pthread_mutex_unlock (&logs);

	listen (servidor, SOMAXCONN);

	FD_SET (servidor, &general);

	maximofd = servidor;

	while (1) {
		temporal = general;

		pthread_mutex_lock (&seleccionador);

		if (select (maximofd + 1, &temporal, NULL, NULL, NULL) == -1) {
			perror ("Falló el select.\n");

			pthread_mutex_lock (&logs);

			log_info (log, "Falló el select ]/.");

			pthread_mutex_unlock (&logs);

			exit (EXIT_FAILURE);
		}

		pthread_mutex_unlock (&seleccionador);

		for (int i = 0; i <= maximofd; i++) {
			pthread_mutex_lock (&dormir);

			sleep (0.5);

			pthread_mutex_unlock (&dormir);

			if (FD_ISSET (i, &temporal)) {
				if (i == servidor) {
					int tamaniodireccion = sizeof (struct sockaddr_in);

					if ((nuevofd = accept (servidor, (void*) &direccioncliente, &tamaniodireccion)) == -1) {
						perror ("Ocurrió un error.\n");

						pthread_mutex_lock (&logs);

						log_info (log, "Falló la conexión a un nuevo cliente.");

						pthread_mutex_unlock (&logs);
					}

					else {
						FD_SET (nuevofd, &general);

						if (nuevofd > maximofd)
							maximofd = nuevofd;

						pthread_t conectador;

						pthread_mutex_lock (&mcliente);

						pthread_mutex_lock (&conectando);

						pthread_create (&conectador, NULL, conectar, &nuevofd);

						pthread_detach (conectador);
					}
				}

				else {
					if (buscarprograma (i)) {
						pthread_mutex_lock (&conectando);

						free (mensaje);

						mensaje = malloc (1);

						if (recv (i, mensaje, 1, 0) <= 0) {
							programa = buscarprograma (i);

							pthread_mutex_unlock (&conectando);

							if (programa -> planificando)
								pthread_mutex_lock (programa -> mutex);

							pthread_mutex_lock (&programases);

							recorrido = programas;

							ult = programa -> ults;

							pthread_mutex_lock (&logs);

							log_info (log, "El cliente %i se desconectó.", i);

							pthread_mutex_unlock (&logs);

							while (ult) {
								aux = ult -> sig;

								pthread_mutex_lock (&logs);

								log_info (log, "Se liberó la memoria del ult %i del programa %i.", ult -> tid, i);

								pthread_mutex_unlock (&logs);

								if (ult -> estado == Ejecutando) {
									if (gradodemultiprogramacion + 1 < config_get_int_value (config, "Multiprogramación")) {
											gradodemultiprogramacion++;

											pthread_mutex_unlock (&multiprogramacion);
									}

									pthread_mutex_lock (&multio);

									multi--;

									pthread_mutex_unlock (&multio);
								}

								//free (ult);

								ult = aux;
							}

							if (recorrido -> pid == programa -> pid)
								programas = programa -> sig;

							else {
								while (recorrido -> sig -> pid != programa -> pid)
									recorrido = recorrido -> sig;

								recorrido -> sig = programa -> sig;
							}

							pthread_mutex_unlock (&programases);

							pthread_mutex_lock (&logs);

							log_info (log, "Se liberó la memoria del programa %i.", programa -> pid);

							pthread_mutex_unlock (&logs);

							//free (programa -> bloqueadoes);

							//free (programa -> mutex);

							//free (programa -> hilos);

							//free (programa -> join);

							//free (programa -> ult0);

							//free (programa);

							programa = NULL;

							close (i);

							FD_CLR (i, &general);
						}

						else {
							pthread_mutex_unlock (&conectando);

							tamaniomensaje = *mensaje;

							if (tamaniomensaje) {
								mensaje = realloc (mensaje, tamaniomensaje + 1);

								mensaje [tamaniomensaje] = '\0';

								if (recv (i, mensaje, tamaniomensaje, 0) > 0) {
									pthread_t atendedorcontroladoporvosje;

									parametros = malloc (sizeof (struct parametros));

									parametros -> request = strdup (mensaje);

									parametros -> cliente = i;

									pthread_mutex_lock (&conectando);

									pthread_create (&atendedorcontroladoporvosje, NULL, atender, parametros);

									pthread_detach (atendedorcontroladoporvosje);

									pthread_mutex_unlock (&conectando);
								}
							}
						}
					}
				}
			}
		}
	}

	free (mensaje);

	free (parametros);

	close (servidor);

	exit (EXIT_SUCCESS);
}

void main () {
	pthread_t ciclador;

	config = config_create ("Config.config");

	log = log_create ("Log.log", "SUSE.c", 1, LOG_LEVEL_INFO);

	pthread_mutex_init (&logs, NULL);

	gradodemultiprogramacion = config_get_int_value (config, "Multiprogramación");

	pthread_mutex_init (&multiprogramacion , NULL);

	pthread_mutex_lock (&multiprogramacion);

	pthread_mutex_init (&seleccionador, NULL);

	pthread_mutex_init (&finalizadoses, NULL);

	pthread_mutex_init (&programases, NULL);

	pthread_mutex_init (&semaforoses, NULL);

	pthread_mutex_init (&mcliente, NULL);

	pthread_mutex_init (&conectando, NULL);

	pthread_mutex_init (&multio, NULL);

	pthread_mutex_init (&dormir, NULL);

	pthread_create (&ciclador, NULL, ciclo, NULL);

	pthread_detach (ciclador);

	suse_init ();

	despertar ();
}
