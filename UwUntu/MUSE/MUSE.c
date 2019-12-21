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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <semaphore.h>
#include <stdbool.h>
#include <semaphore.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <sys/mman.h>

typedef enum {
	HEAP, MAP
}type;

typedef enum {
	PRIVATE, SHARED
}flag;

typedef struct {
	int LISTEN_PORT;
	int MEMORY_SIZE;
	int PAGE_SIZE;
	int SWAP_SIZE;
	char* ip;
} archivoConfiguracion;

typedef enum {
	MP, SWAP
} ubicacion;

typedef struct {
	bool bitPrecencia;
	int nroframe;
	ubicacion ubicacion;
	uint32_t desplazamiento;
} pagina;

typedef struct {
	int ID;
	t_list* tablaSegmentos;
} proceso;

typedef enum {
	FREE, USED
} estado;

typedef struct {
	uint32_t baseLogica;
	uint32_t tamano;
	type tipo;
	t_list* tablaPaginas;
	t_list* metadatas;
} segmento;

typedef struct {
	int PID;
	pagina* pagina;
	bool bitUso;
	bool bitMod;
	bool enUso;
} frame;

typedef struct{
	char* path;
	flag compartido;
	int instancias;
	t_list* paginas;
	void* map;
}archivo;

typedef struct {
	bool ocupado;
	pagina* pagina;
} swap;

typedef struct {
	int sd;
	float porcentajeAsignado;		//cantidad de segmentos asignados dividido segmentos totales
	float memoriaDisponible;		//cantidad de segmentos asignados dividido segmentos totales
} logSocket;

typedef struct {
	int programId;
	float memoriaPedida;
	float memoriaLiberada;
	float memoriaLeak;
} logPrograma;

typedef struct{
	bool ocupado;
	uint32_t tam;
	uint32_t dir;
}metadata;


void configurar();
int muse_init(int);
void agregarUsuario(int);
void muse_close(int);
void quitarUsuario(int);
uint32_t muse_alloc(int, uint32_t);
proceso* obtenerProceso(int);
uint32_t crearNuevoSegmento(t_list*, uint32_t, type, int);
uint32_t segUltimaPosicion(t_list*);
bool sePuedeAgrandarSegmento(t_list*, uint32_t, type);
int segmentoAgrandar(t_list*, uint32_t, type);
int obtenerFrameLibre();
uint32_t agrandarSegmento(t_list*, uint32_t, type, int);
int obtenerFrame(pagina*, int);
void muse_free(int, uint32_t);
uint32_t getMetadataTam(int, uint32_t);
bool getMetadataEstado(int, uint32_t);
int muse_copy(int, uint32_t, void*, int);
int muse_get(int, void*, uint32_t, int);
void loggearConfig();
void loggearError(int, int);
void mandarInt(int, int);
int recibirInt(int);
uint32_t muse_map(int, char *, size_t, int);
uint32_t mappearArchivo(int, char*, uint32_t, archivo*);
int muse_sync(int, uint32_t, uint32_t);
int clockModificado();
void moverPunteroEnClock();
int escribirMMap(pagina*);
int leerMMap(pagina*);
void mandarInt(int sd, int valor);
void* recibirVoid(int sd);
void mandarVoid(int sd, void* valor, int n);
int muse_unmap(int id, uint32_t dir);
void loggear();

char* memoriaPrincipal;
frame* frames;
t_list* procesos;
archivoConfiguracion config;
t_list* archivosMappeados;
swap* swaps;
int logSize=30;
logSocket logsSockets[30];
logPrograma logsProgramas[30];
void *map;
int punteroClock = 0;

#define FILESIZE (config.SWAP_SIZE)
#define NUMINTS  (FILESIZE/config.PAGE_SIZE)
#define FILEPATH ("./Swap.bin")

int main(int argc, char *argv[]) {

	configurar();

	char* mp = malloc(config.MEMORY_SIZE);

	memoriaPrincipal = mp;

	frame vectorFrames[config.MEMORY_SIZE / config.PAGE_SIZE];

	swap superSwaps[config.SWAP_SIZE / config.PAGE_SIZE];



	for (int i = 0; i<30; i++){
		logsSockets[i].sd= -1;
		logsProgramas[i].programId= -1;
		logsSockets[i].memoriaDisponible=0;
		logsSockets[i].porcentajeAsignado=0;
		logsProgramas[i].memoriaLeak=0;
		logsProgramas[i].memoriaLiberada=0;
		logsProgramas[i].memoriaPedida=0;
	}

	for(int i = 0; i < config.SWAP_SIZE / config.PAGE_SIZE; i++)
	{
		superSwaps[i].ocupado = false;
	}

	for(int i = 0; i < config.MEMORY_SIZE / config.PAGE_SIZE; i++)
	{
		vectorFrames[i].enUso = false;
	}

	swaps = superSwaps;

	frames = vectorFrames;

	procesos = list_create();

	archivosMappeados = list_create();

	int opt = 1;
	int master_socket,addrlen, new_socket, client_socket[30], max_clients = 30, activity, i, sd;
	int max_sd;
	struct sockaddr_in address;

	fd_set readfds;

	for(i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
	}

	if((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("Fallo la creacion del socket");
		exit(EXIT_FAILURE);
	}

	if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char*) &opt, sizeof(opt)) < 0)
	{
		perror("Fallo el seteo de propiedades del socket");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("172.17.0.1");
	address.sin_port = htons(config.LISTEN_PORT);

	if(bind(master_socket, (struct sockaddr *) &address, sizeof(address)) < 0)
	{
		perror("Fallo el Bind");
		exit(EXIT_FAILURE);
	}

	listen(master_socket, 100);

	addrlen = sizeof(address);

	while(1)
	{
		FD_ZERO(&readfds);

		FD_SET(master_socket, &readfds);

		max_sd = master_socket;

		for(i = 0; i < max_clients; i++)
		{
			sd = client_socket[i];

			if(sd > 0)
			{
				FD_SET(sd, &readfds);
			}

			if(sd > max_sd)
			{
				max_sd = sd;
			}
		}

		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if((activity < 0) && (errno != EINTR))
		{
			perror("Error en el Select");
		}

		if(FD_ISSET(master_socket, &readfds))
		{
			new_socket = accept(master_socket, (struct sockaddr *) &address, (socklen_t*) &addrlen);

			if(new_socket < 0)
			{
					perror("Accept");
					exit(EXIT_FAILURE);
			}

			for(i = 0; i < max_clients; i++)
			{
				if(client_socket[i] == 0)
				{
					client_socket[i] = new_socket;

					break;
				}
			}
		}

		for(i = 0; i < max_clients; i++)
		{
			sd = client_socket[i];

			if(FD_ISSET(sd, &readfds))
			{
				int operacion = recibirInt(sd);
				if(operacion == 0)
				{
					getpeername(sd, (struct sockaddr *) &address, (socklen_t *) &addrlen);
					close(sd);
					client_socket[i] = i;
				}else
				{
					switch(operacion)
					{
						case 1:
							;

							int ID = recibirInt(sd);

							int res = muse_init(ID);

							mandarInt(sd, res);

							break;

						case 2:
							;

							int ID8 = recibirInt(sd);

							muse_close(ID8);

							break;

						case 3:
							;

							int ID9 = recibirInt(sd);

							uint32_t tam9 = recibirInt(sd);

							mandarInt(sd, muse_alloc(ID9, tam9));

							break;

						case 4:
							;

							int ID2 = recibirInt(sd);

							uint32_t dir2 = recibirInt(sd);

							muse_free(ID2, dir2);

							break;

						case 5:
							;

							int ID3 = recibirInt(sd);

							void* valor = recibirVoid(sd);

							uint32_t dir3 = recibirInt(sd);

							uint32_t tam3 = recibirInt(sd);

							int error = muse_get(ID3, valor, dir3, tam3);

							mandarInt(sd, error);

							mandarVoid(sd, valor, tam3);

							free(valor);
							valor = NULL;

							break;

						case 6:
							;

							int ID4 = recibirInt(sd);

							void* contenido = recibirVoid(sd);

							uint32_t dir4 = recibirInt(sd);

							uint32_t tam4 = recibirInt(sd);

							int error2 = muse_copy(ID4, dir4, contenido, tam4);

							mandarInt(sd, error2);

							free(contenido);

							break;

						case 7:
							;

							int ID5 = recibirInt(sd);

							char* path = recibirVoid(sd);

							size_t size = recibirInt(sd);

							flag flags = recibirInt(sd);

							mandarInt(sd, muse_map(ID5, path, size, flags));

							break;

						case 8:
							;

							int ID6 = recibirInt(sd);

							uint32_t dir6 = recibirInt(sd);

							uint32_t tam6 = recibirInt(sd);

							mandarInt(sd, muse_sync(ID6, dir6, tam6));

							break;

						case 9:
							;

							int ID7 = recibirInt(sd);

							uint32_t dir7 = recibirInt(sd);

							mandarInt(sd, muse_unmap(ID7, dir7));

							break;
					}
				}
			}
		}
	}
}

void configurar() 
{
	t_config* arch = config_create("Config.config");

	config.LISTEN_PORT = config_get_int_value(arch, "LISTEN_PORT");
	config.MEMORY_SIZE = config_get_int_value(arch, "MEMORY_SIZE");
	config.PAGE_SIZE = config_get_int_value(arch, "PAGE_SIZE");
	config.SWAP_SIZE = config_get_int_value(arch, "SWAP_SIZE");

	config_destroy(arch);

	int fd;
	int result;

	fd = open(FILEPATH, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
	if (fd == -1) 
	{
		perror("Error opening file for writing");
		exit(EXIT_FAILURE);
	}


	result = lseek(fd, FILESIZE-1, SEEK_SET);
	if (result == -1)
	{
		close(fd);
		perror("Error calling lseek() to 'stretch' the file");
		exit(EXIT_FAILURE);
	}

	result = write(fd, "", 1);
	if (result != 1) 
	{
		close(fd);
		perror("Error writing last byte of the file");
		exit(EXIT_FAILURE);
	}

	map = mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
	{
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
	}
}

void loggearError(int id, int codigo)
{
	t_log* log = log_create("Logs", "MUSE", 1, LOG_LEVEL_ERROR);

	char* mensaje = malloc(50);

	sprintf(mensaje, "Error: %d, Proceso: %d", codigo, id);

	log_error(log, mensaje);

	log_destroy(log);
	free(mensaje);
}

void loggearConfig()
{
	t_log* log = log_create("Logs", "MUSE", 1, LOG_LEVEL_INFO);

	char* mensaje = malloc(50);

	strcpy(mensaje, "Se termino la configuracion");

	log_info(log, mensaje);

	log_destroy(log);
	free(mensaje);
}

int muse_init(int id) {

		agregarUsuario(id);

		return 0;
}

void agregarUsuario(int id) {

	proceso* prog = malloc(sizeof(proceso));
	prog->ID = id;
	prog->tablaSegmentos = list_create();

	list_add(procesos, prog);
}

void muse_close(int id)
{
	quitarUsuario(id);
}

void quitarUsuario(int id) {

	int condicion(proceso* proceso) {

		if (proceso->ID == id)
		{
			for(int i = 0; i < list_size(proceso->tablaSegmentos); i++)
			{
				segmento* seg = list_get(proceso->tablaSegmentos, i);
				for(int j = 0; j < list_size(seg->tablaPaginas); j++)
				{
					pagina* pag = list_get(seg->tablaPaginas, j);
					(frames + pag->nroframe)->enUso = false;
				}
			}
			return 1;
		}

		return 0;
	}

	void destroyer(void* proceso)
	{
		free(proceso);
	}

	list_remove_and_destroy_by_condition(procesos, (void*) condicion, (void*) destroyer);
}

uint32_t muse_alloc(int id, uint32_t tam)
{
	proceso* prog = obtenerProceso(id);

	if (sePuedeAgrandarSegmento(prog->tablaSegmentos, tam, HEAP))

		return agrandarSegmento(prog->tablaSegmentos, tam, HEAP, id);

	else
	{
		return crearNuevoSegmento(prog->tablaSegmentos, tam, HEAP, id);
	}
}

proceso* obtenerProceso(int id) {

	int condicion(proceso* proceso) {

		if (proceso->ID == id)
			return 1;

		return 0;
	}

	proceso* proceso = list_find(procesos, (void*) condicion);

	return proceso;
}

int segmentoAgrandar(t_list* segs, uint32_t tam, type tipo)
{
	bool comparator(segmento* elemento, segmento* elementoSig)
	{
		if(elemento->baseLogica < elementoSig->baseLogica)
			return true;

		return false;
	}

	list_sort(segs, (void*) comparator);

	for (int i = 0; i < list_size(segs); i++)
	{
		segmento* actual = list_get(segs, i);

		if (list_get(segs, i + 1) == 0 && actual->tipo == tipo)
			return i;

		segmento* sig = list_get(segs, i + 1);

		if ((actual->baseLogica + actual->tamano + tam) < sig->baseLogica && actual->tipo == tipo)
			return i;
	}

	return -1;
}

bool sePuedeAgrandarSegmento(t_list* segs, uint32_t tam, type tipo)
{
	if (segmentoAgrandar(segs, tam, tipo) != -1)

		return true;

	return false;
}

int obtenerFrameLibre()
{
	for (int i = 0; i < config.MEMORY_SIZE / config.PAGE_SIZE; i++)
	{
		if ((frames + i)->enUso == false)
		{
			(frames + i)->enUso = true;
			return i;
		}
	}

	int num = clockModificado();

	escribirMMap((frames + num)->pagina);
	(frames + num)->pagina->ubicacion = SWAP;

	return num;
}

pagina* crearPagina(int id, segmento* seg, int desplazamiento)
{
	int frameLibre = obtenerFrameLibre();

	pagina* pag = malloc(sizeof(pagina));
	pag->bitPrecencia = true;
	pag->nroframe = frameLibre;
	pag->ubicacion = MP;
	pag->desplazamiento = desplazamiento;

	list_add(seg->tablaPaginas, pag);

	(frames + frameLibre)->PID = id;
	(frames + frameLibre)->pagina = pag;
	(frames + frameLibre)->bitMod = true;
	(frames + frameLibre)->bitUso = true;

	return pag;
}

int obtenerFrame(pagina* pag, int id)
{
	if(pag->ubicacion == MP)
	{
		for(int i = 0; i < config.MEMORY_SIZE / config.PAGE_SIZE; i++)
		{
			if((frames+i)->PID == id && (frames+i)->pagina == pag)
				return i;
		}
	}
	else
	{
		int num = clockModificado();

		pag->nroframe = num;

		escribirMMap((frames + num)->pagina);
		(frames + num)->pagina->ubicacion = SWAP;

		leerMMap(pag);
		pag->ubicacion = MP;

		(frames + num)->pagina = pag;
		(frames + num)->bitMod = false;

		return num;
	}

	return -1;
}

uint32_t agrandarSegmento(t_list* tablaSegmentos, uint32_t tam, type tipo, int id)
{
	int i = 0;

	segmento* seg = list_get(tablaSegmentos, segmentoAgrandar(tablaSegmentos, tam, tipo));

	uint32_t retorno = seg->baseLogica + seg->tamano;

	seg->tamano += tam;

	pagina* pag = list_get(seg->tablaPaginas, list_size(seg->tablaPaginas)-1);

	if(pag->desplazamiento < config.PAGE_SIZE)
	{
		metadata* metadata = malloc(sizeof(metadata));
		metadata->ocupado = false;
		metadata->tam = tam;
		metadata->dir = retorno;

		list_add(seg->metadatas, metadata);
		i++;

		if((config.PAGE_SIZE - pag->desplazamiento) > tam)
			tam = 0;
		else
			tam -= (config.PAGE_SIZE - pag->desplazamiento);

		pag->desplazamiento = config.PAGE_SIZE;
	}

	int ocupado = 0;

	while(tam > 0)
	{
		if(tam > config.PAGE_SIZE)
			ocupado = config.PAGE_SIZE;
		else
			ocupado = tam;

		crearPagina(id, seg, ocupado);

		if(i == 0)
		{
			metadata* metadata = malloc(sizeof(metadata));
			metadata->ocupado = false;
			metadata->tam = tam;
			metadata->dir = retorno;

			list_add(seg->metadatas, metadata);
			i++;
		}

		tam -= ocupado;
	}

	return retorno;
}

uint32_t segUltimaPosicion(t_list* listaSegmentos)
{
	if (list_size(listaSegmentos) != 0)
	{
		segmento* seg = list_get(listaSegmentos, list_size(listaSegmentos) - 1);

		return seg->baseLogica + seg->tamano;

	}else

		return 0;
}

uint32_t crearNuevoSegmento(t_list* listaSegmentos, uint32_t tam, type Tipo, int id)
{
	proceso* prog = obtenerProceso(id);

	segmento* seg = malloc(sizeof(segmento));
	seg->baseLogica = segUltimaPosicion(listaSegmentos);
	seg->tamano = tam;
	seg->tipo = Tipo;
	seg->tablaPaginas = list_create();
	seg->metadatas = list_create();

	int ocupado = 0;
	int i = 0;

	while(tam > 0)
	{
		if(tam > config.PAGE_SIZE)
			ocupado = config.PAGE_SIZE;
		else
			ocupado = tam;

		crearPagina(id, seg, ocupado);

		if(i == 0)
		{
			metadata* metadata = malloc(sizeof(metadata));
			metadata->ocupado = false;
			metadata->tam = tam;
			metadata->dir = seg->baseLogica;

			list_add(seg->metadatas, metadata);
			i++;
		}

		tam -= ocupado;
	}

	list_add(prog->tablaSegmentos, seg);

	return seg->baseLogica;
}

void muse_free(int id, uint32_t dir)
{
	bool obtenerSegmento(segmento* elemento)
	{
		if(elemento->baseLogica <= dir && (elemento->baseLogica + elemento->tamano) >= dir)
			return true;

		return false;
	}

	bool obtenerMetadata(metadata* elemento)
	{
		if(elemento->dir == dir)
			return true;

		return false;
	}

	proceso* proceso = obtenerProceso(id);

	segmento* segmento = list_find(proceso->tablaSegmentos, (void*) obtenerSegmento);

	metadata* metadata = list_find(segmento->metadatas, (void*) obtenerMetadata);

	if(segmento->baseLogica == dir)
		segmento->baseLogica += metadata->tam;
}

bool getMetadataEstado(int id, uint32_t dir)
{
	proceso* proceso = obtenerProceso(id);

	bool obtenerSegmento(segmento* elemento)
	{
		if(elemento->baseLogica <= dir && (elemento->baseLogica + elemento->tamano) > dir)
			return true;

		return false;
	}

	segmento* seg = list_find(proceso->tablaSegmentos, (void*) obtenerSegmento);

	uint32_t dirPag = dir - seg->baseLogica;

	int nroPag = dirPag / config.PAGE_SIZE;

	pagina* pag = list_get(seg->tablaPaginas, nroPag);

	uint32_t desplazamiento;

	if(dirPag % config.PAGE_SIZE  != 0)
	{
		desplazamiento = dirPag - (nroPag * config.PAGE_SIZE);

	}else
	{
		desplazamiento = 0;
	}

	bool* estado = malloc(sizeof(bool));
	memcpy(estado, memoriaPrincipal + desplazamiento +(pag->nroframe * config.PAGE_SIZE), sizeof(uint32_t));

	bool respuesta = *estado;
	free(estado);

	return respuesta;
}

uint32_t getMetadataTam(int id, uint32_t dir)
{
	proceso* proceso = obtenerProceso(id);

	bool obtenerSegmento(segmento* elemento)
	{
		if(elemento->baseLogica <= dir && (elemento->baseLogica + elemento->tamano) > dir)
			return true;

		return false;
	}

	segmento* seg = list_find(proceso->tablaSegmentos, (void*) obtenerSegmento);

	if(seg == NULL)
		return -1;

	uint32_t dirPag = dir - seg->baseLogica;

	int nroPag = dirPag / config.PAGE_SIZE;

	pagina* pag = list_get(seg->tablaPaginas, nroPag);

	if(pag->ubicacion == SWAP)
	{
		int num = clockModificado();

		pag->nroframe = num;

		(frames + num)->pagina->ubicacion = SWAP;
		escribirMMap((frames + num)->pagina);

		leerMMap(pag);
		pag->ubicacion = MP;

		(frames + num)->pagina = pag;
		(frames + num)->bitMod = false;
	}

	uint32_t desplazamiento = dirPag % config.PAGE_SIZE;

	uint32_t* tam = malloc(sizeof(uint32_t));
	memcpy(tam, memoriaPrincipal + desplazamiento + 1 +(pag->nroframe * config.PAGE_SIZE), sizeof(uint32_t));

	uint32_t respuesta = *tam;
	free(tam);

	return respuesta;
}


int muse_copy(int id, uint32_t dir, void* contenido, int n)
{
	proceso* proceso = obtenerProceso(id);

	bool obtenerSegmento(segmento* elemento)
	{
		if(elemento->baseLogica <= dir && (elemento->baseLogica + elemento->tamano) >= dir)
			return true;

		return false;
	}

	segmento* seg = list_find(proceso->tablaSegmentos, (void*) obtenerSegmento);

	if(seg == NULL)
		return -1;

	uint32_t dirPag = dir - seg->baseLogica;

	int nroPag = dirPag / config.PAGE_SIZE;

	uint32_t desplazamiento = dirPag % config.PAGE_SIZE;

	if((dir + n) <= (seg->baseLogica + seg->tamano))
	{
		int i = 0;
		int copiado = 0;
		int aCopiar = 0;

		while(n > 0)
		{
			pagina* pag = list_get(seg->tablaPaginas, nroPag + i);

			if(pag->ubicacion == SWAP)
			{
				int num = clockModificado();
				pag->nroframe = num;

				(frames + num)->pagina->ubicacion = SWAP;
				escribirMMap((frames + num)->pagina);

				pag->ubicacion = MP;
				(frames + num)->pagina = pag;
				leerMMap(pag);
			}

			if(i == 0)
			{
				aCopiar = pag->desplazamiento - desplazamiento;

				if(n < aCopiar)
					aCopiar = n;
			}
			else
			{
				if(n > pag->desplazamiento)
					aCopiar = pag->desplazamiento;
				else
				{
					aCopiar = n;
				}
			}

			memcpy((memoriaPrincipal + (pag->nroframe * config.PAGE_SIZE) + desplazamiento), (contenido + copiado), aCopiar);

			copiado += aCopiar;
			n -= aCopiar;
			i++;
			desplazamiento = 0;
		}

		return 0;

	}else
	{
		return -1;
	}
}

int muse_get(int id, void* destino, uint32_t dir, int n)
{
		proceso* prog = obtenerProceso(id);

		bool obtenerSegmento(segmento* elemento)
		{
			if(elemento->baseLogica <= dir && (elemento->baseLogica + elemento->tamano) >= dir)
				return true;

				return false;
		}

		segmento* seg = list_find(prog->tablaSegmentos, (void*) obtenerSegmento);

		if(seg == NULL)
		{
			printf("Seg no encontrado\n");
			return -1;
		}
		uint32_t dirPag = dir - seg->baseLogica;

		int nroPag = dirPag / config.PAGE_SIZE;

		uint32_t desplazamiento = dirPag % config.PAGE_SIZE;

		if((dir + n) <= (seg->baseLogica + seg->tamano))
		{
			int i = 0;
			int copiado = 0;
			int aCopiar = 0;

			while(n > 0)
			{
				pagina* pag = list_get(seg->tablaPaginas, nroPag + i);

				if(pag->ubicacion == SWAP)
				{
					int num = clockModificado();
					pag->nroframe = num;

					(frames + num)->pagina->ubicacion = SWAP;
					escribirMMap((frames + num)->pagina);

					pag->ubicacion = MP;
					(frames + num)->pagina = pag;
					leerMMap(pag);
				}

				if(i == 0)
				{
					aCopiar = pag->desplazamiento - desplazamiento;

					if(n < aCopiar)
						aCopiar = n;
				}
				else
				{
					if(n > pag->desplazamiento)
						aCopiar = pag->desplazamiento;
					else
					{
						aCopiar = n;
					}
				}

				memcpy((destino + copiado), (memoriaPrincipal + (pag->nroframe * config.PAGE_SIZE) + desplazamiento), aCopiar);

				copiado += aCopiar;
				n -= aCopiar;
				i++;
				desplazamiento = 0;
			}
		}

	char* algo = malloc(2046);
	memcpy(algo, destino, 2046);
	return 0;
}

uint32_t muse_map(int id, char *path, size_t length, int flags)
{
	bool buscarArchivo(archivo* elemento)
	{
		if(!strcmp(path, elemento->path))
			return true;

		return false;
	}

		proceso* prog = obtenerProceso(id);
		archivo* arch = list_find(archivosMappeados, (void*) buscarArchivo);

		if(arch == NULL)
		{
			arch = malloc(sizeof(archivo));
			arch->compartido = flags;
			arch->instancias = 1;
			arch->path = path;

			list_add(archivosMappeados, arch);

			return mappearArchivo(id, path, length, arch);
		}
		else
		{
			if(arch->compartido == SHARED)
			{
				arch->instancias += 1;

				segmento* seg = malloc(sizeof(segmento));
				seg->baseLogica = segUltimaPosicion(prog->tablaSegmentos);
				seg->tamano = length;
				seg->tipo = MAP;
				seg->tablaPaginas = arch->paginas;

				list_add(prog->tablaSegmentos, seg);

				return seg->baseLogica;
			}
			else
			{
				loggearError(id, 8);
				return -1;
			}
		}
}

uint32_t mappearArchivo(int id, char* path, uint32_t length, archivo* arch)
{
	int fd;
	uint32_t dir;
	segmento* seg;
	void* muse_m;

	bool segmento(segmento* elemento)
	{
		if(elemento->baseLogica == dir)
			return true;

		return false;
	}

	proceso* prog = obtenerProceso(id);

	int result;

    fd = open(path, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
    	perror("Error opening file for writing");
    	exit(EXIT_FAILURE);
    }


    result = lseek(fd, length-1, SEEK_SET);
    if (result == -1) {
    	close(fd);
    	perror("Error calling lseek() to 'stretch' the file");
    	exit(EXIT_FAILURE);
    }


    result = write(fd, "", 1);
    if (result != 1) {
    	close(fd);
    	perror("Error writing last byte of the file");
    	exit(EXIT_FAILURE);
    }

    muse_m = mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
    	close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
    }

	dir =  crearNuevoSegmento(prog->tablaSegmentos, length, MAP, id);

	seg = list_find(prog->tablaSegmentos, (void*) segmento);

	arch->paginas = seg->tablaPaginas;
	arch->map = muse_m;

	//muse_copy(id, dir, muse_m, length);

	return dir;
}

int muse_sync(int id, uint32_t dir, uint32_t len)
{
		proceso* prog = obtenerProceso(id);

		bool obtenerSegmento(segmento* elemento)
		{
			if(elemento->baseLogica <= dir && (elemento->baseLogica + elemento->tamano) >= dir)
				return true;

			return false;
		}

		segmento* seg = list_find(prog->tablaSegmentos, (void*) obtenerSegmento);

		if(seg == NULL)
			return -1;

		bool obtenerArchivo(archivo* elemento)
		{
			if(elemento->paginas == seg->tablaPaginas)
				return true;

			return false;
		}

		if(seg->tipo == MAP)
		{
			archivo* arch = list_find(archivosMappeados, (void*) obtenerArchivo);
			char* inter = malloc(len);
			muse_get(id, inter, seg->baseLogica + 4, len - 4);

			memcpy(arch->map, inter, len);
			msync(arch->map, seg->tamano, MS_SYNC);
		}
		else
		{
			loggearError(id, 9);
			return -1;
		}

		return 0;

}

int clockModificado()
{

	for (int ciclosClock = 0; ciclosClock < 4;ciclosClock++ )
		{
			if (ciclosClock ==0 || ciclosClock ==2 )
			{
				int pasajesNecesarios = config.MEMORY_SIZE / config.PAGE_SIZE;

				for (int i = punteroClock;  i < config.MEMORY_SIZE / config.PAGE_SIZE && pasajesNecesarios>0; i++)
					{

						if ((frames + i)->bitUso == 0 && (frames + i)->bitMod == 0)
							{
							moverPunteroEnClock(i);
								return i;
							}
						if (i==config.MEMORY_SIZE / config.PAGE_SIZE-1 && pasajesNecesarios>0){
							i=-1;
						}
						pasajesNecesarios--;

					}

			}
			if (ciclosClock ==1 || ciclosClock ==3)
				{
				int pasajesNecesarios = config.MEMORY_SIZE / config.PAGE_SIZE;

				for (int i = punteroClock;  i < config.MEMORY_SIZE / config.PAGE_SIZE && pasajesNecesarios>0; i++)
					{
						if ((frames + i)->bitUso == 0 && (frames + i)->bitMod == 1)
							{
								moverPunteroEnClock(i);
								return i;
							}
						else
						{
							(frames + i)->bitUso = false;
						}
						if (i==config.MEMORY_SIZE / config.PAGE_SIZE-1 && pasajesNecesarios>0){
							i=-1;
						}
						pasajesNecesarios--;
					}

			}

	}
	return 0;
}

void moverPunteroEnClock (int puntero){
	if (puntero+1>=config.MEMORY_SIZE / config.PAGE_SIZE){
		punteroClock=0;
	}
	else {
		punteroClock = puntero+1;
	}
}

int escribirMMap(pagina* unaPag)
{


    /* Now write int's to the file as if it were memory (an array of ints).
     */
    int posicionAEscribir =-1;
    int nroFrame = unaPag->nroframe;

    for (int j = 0; j < NUMINTS; j++){
        	if ((swaps + j)->pagina == unaPag){
        		perror("mandaste a swap algo que ya estaba ahi");
        	}
        }


    for (int j = 0; j < NUMINTS; j++){
    	if (!(swaps + j)->ocupado)
    	{
    		posicionAEscribir = j;
    		j = NUMINTS;
    	}
    }

    if (posicionAEscribir == -1){
    	perror("mandaste a algo a swap mientras estaba lleno");
    }

    memcpy(map +posicionAEscribir*config.PAGE_SIZE, memoriaPrincipal + (nroFrame * config.PAGE_SIZE),config.PAGE_SIZE);


    (swaps+posicionAEscribir)->pagina = unaPag;
    (swaps+posicionAEscribir)->ocupado = true;

    msync(map, config.PAGE_SIZE, MS_SYNC);

    return 0;
}

int leerMMap(pagina* unaPag)
{

	    /* Read the file int-by-int from the mmap
	     */
	    int posicionALeer =-1;
	    for (int j = 0; j< NUMINTS; j++){
	       if((swaps + j)->pagina == unaPag && (swaps + j)->ocupado){
	    	   posicionALeer =j;
	        }
	    }
	    if (posicionALeer==-1){
	    	perror("mandaste a buscar a swap algo que no estaba ahi");
	    }

	    int nroFrame = unaPag->nroframe;
	    memcpy( memoriaPrincipal + (nroFrame * config.PAGE_SIZE),map +posicionALeer*config.PAGE_SIZE,config.PAGE_SIZE);


	    (swaps+posicionALeer)->ocupado = false;
	    (swaps+posicionALeer)->pagina = NULL;


	    //msync(map, config.SWAP_SIZE, MS_SYNC);

	    return 0;
}

int muse_unmap(int id, uint32_t dir)
{
		proceso* prog = obtenerProceso(id);

		bool obtenerSegmento(segmento* elemento)
		{
			if(elemento->baseLogica <= dir && (elemento->baseLogica + elemento->tamano) >= dir)
				return true;

				return false;
		}

		segmento* seg = list_find(prog->tablaSegmentos, (void*) obtenerSegmento);

		if(seg == NULL)
			return -1;

		if(seg->tipo == HEAP)
		{
			loggearError(id, 10);

			return -1;
		}

		bool buscarArchivo(archivo* elemento)
		{
			if(elemento->paginas == seg->tablaPaginas)
				return true;

			return false;
		}

		archivo* arch = list_find(archivosMappeados, (void*) buscarArchivo);

		arch->instancias -= 1;

		if(arch->instancias < 1)
			munmap(arch->map, seg->tamano);

		list_remove_by_condition(prog->tablaSegmentos, (void*) obtenerSegmento);

		return 0;
}

void mandarInt(int sd, int valor)
{
	send(sd, &valor, sizeof(int), 0);
}

int recibirInt(int sd)
{
	int resultado, retorno;

	resultado = recv(sd, &retorno, sizeof(int), 0);

	if(resultado == 0)
	{
		return resultado;
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

	void* stream = malloc(longitud + 100);

	recv(sd, stream, longitud, 0);

	return stream;
}



void crearLogPrograma(int id){
	int i=0;
	while(logsProgramas[i].programId!=-1){
		i++;
	}
	logsProgramas[i].programId=id;

}

void crearLogSocket(int id){
	int i=0;
	while(logsSockets[i].sd!=-1){
		i++;
	}
	logsSockets[i].sd=id;

}


void loggear()
{
	t_log* log = log_create("Metricas", "MUSE", 1, LOG_LEVEL_INFO);

	char* mensaje = malloc(500);
	int i = 0;
	while(logsProgramas[i].programId!=-1 && i<logSize){
		sprintf(mensaje, "Programa: %d \n	memoriaPedida: %f \n	memoriaLiberada: %f \n	memoryLeak: %f \n\n", logsProgramas[i].programId,logsProgramas[i].memoriaPedida,logsProgramas[i].memoriaLiberada,logsProgramas[i].memoriaLeak);

	}
	log_info(log, mensaje);
	free(mensaje);
	char* mensaje2 = malloc(500);
	while(logsSockets[i].sd!=-1 && i<logSize){
		sprintf(mensaje, "Socket: %d \n		memoriaDisponible: %f \n	PorcentajeAsignado: %f \n\n",logsSockets[i].sd,logsSockets[i].memoriaDisponible,logsSockets[i].porcentajeAsignado );

	}
	log_info(log, mensaje2);
	log_destroy(log);
	free(mensaje);
}
