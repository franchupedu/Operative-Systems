#include <commons/config.h>
#include <commons/log.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifndef SUSE_H_
#define SUSE_H_

pthread_mutex_t multiprogramacion;

pthread_mutex_t seleccionador;

pthread_mutex_t finalizadoses;

int gradodemultiprogramacion;

pthread_mutex_t semaforoses;

pthread_mutex_t programases;

pthread_mutex_t finalizando;

pthread_mutex_t conectando;

pthread_mutex_t mcliente;

pthread_mutex_t multio;

pthread_mutex_t dormir;

pthread_mutex_t logs;

t_config *config;

int multi = 0;

t_log *log;

struct finalizado *finalizados = NULL;

struct semaforo *semaforos = NULL;

struct programa *programas = NULL;

struct semaforo {
	struct semaforeado *semaforeados;
	struct semaforo *sig;
	char *sid;
	int max;
	int val;
};

struct programa {
	pthread_mutex_t *bloqueadoes;
	pthread_mutex_t *mutex;
	pthread_mutex_t *hilos;
	pthread_mutex_t *join;
	pthread_mutex_t *ult0;
	struct programa *sig;
	char planificando;
	struct ult *ults;
	char tienehilos;
	int ejecutando;
	int bloqueados;
	char bloqueado;
	int joineado;
	int listos;
	char u0;
	char u1;
	int pid;
};

struct ult {
	double estimacion;
	struct ult *sig;
	int ejecucion;
	int creacion;
	int duracion;
	int empieza;
	int inicio;
	int estado;
	int espera;
	char join;
	int tid;
};

struct finalizado {
	struct finalizado *sig;
	int ejecucion;
	int pid;
	int tid;
};

struct semaforeado {
	struct semaforeado *sig;
	int pid;
	int tid;
};

struct parametros {
	char *request;
	int cliente;
};

enum estado {
	Listo,
	Ejecutando,
	Bloqueado
};

enum joineado {
	Nuevo,
	Joineado,
	Liberado
};

#endif
