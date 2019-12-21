#include "sac-cli.h"

/*Funciones de FUSE - Callbacks
 * Crear, escribir y borrar archivos
 * Crear y listar directorios y sus archivos
 * Eliminar directorios
 * Describir directorios y archivos
 * */

void iniciar_logs() {

	loggerINFO = log_create("logs-cliINFO.log", "SAC-CLI", 0, LOG_LEVEL_INFO);
	loggerERROR = log_create("logs.cliERROR.log", "SAC-CLI", 0, LOG_LEVEL_ERROR);
}


int sac_mknod(const char *path, mode_t mode, dev_t dev) {

	log_info(loggerINFO, "*** MKNOD: SAC está creando un archivo *** \n");
	char *respuesta;
	int estado;

	int tam_path_modo = sizeof(int) + strlen(path) + sizeof(int);
	char* path_modo = malloc(tam_path_modo + 1);
	int tam_path = strlen(path);

	int archivo = 2;
	int directorio = 3;

		if (S_ISREG(mode) ) {
			memcpy(path_modo, &archivo, sizeof(int));
		}

		if(S_ISDIR(mode)) {
			memcpy(path_modo, &directorio, sizeof(int));
		}
		memcpy(path_modo + sizeof(int), &tam_path, sizeof(int));
		memcpy(path_modo + 2 * sizeof(int), path, tam_path);

		path_modo[tam_path_modo] = '\0';
		respuesta = sac_crear_nodo(path_modo);

		free (path_modo);

		memcpy(&estado, respuesta, sizeof(int));

		free (respuesta);

		if (estado != 0)
			return -errno;

		return 0;

}


/** Set access and modification time, with nanosecond resolution.
 * The arguments are the number of nanoseconds since jan 1 1970 00:00.**/
int sac_utimens(const char *path, const struct timespec tv[2]) {
	log_info(loggerINFO, "*** UTIMENS: SAC está...  *** \n");
	return 0;
}


static int sac_open(const char *path, struct fuse_file_info *fi) {

	log_info(loggerINFO, "***OPEN: SAC está abriendo un archivo *** \n");
	char *respuesta;
	int estado;
	int nro_nodo;


	respuesta = sac_abrir(path);

	memcpy(&estado, respuesta, sizeof(int));

	if (estado == 1) {
		free (respuesta);
		return -1;
	}

	memcpy(&nro_nodo, respuesta + sizeof(int), sizeof(int));

	free (respuesta);

	return EXIT_SUCCESS;
}


static int sac_access (const char* path, int mode) {

	log_info(loggerINFO, "***ACCESS: SAC está pidiendo acceso a un archivo *** \n");

	struct fuse_file_info *fi;

	int resp = sac_open (path, fi);

	return resp;
}


int sac_write(const char *path, const char *buf, size_t size, off_t offset2,
		struct fuse_file_info *fi) {

	int offset = offset2;
	int tam_path = strlen(path);
	int tam_buf = size;
	int tam_msj = tam_path + tam_buf + 3*sizeof(int);
	//path + buffer + offset
	log_info(loggerINFO, "*** WRITE: SAC está escribiendo un archivo *** \n");
	char* mensaje_server = malloc(tam_msj + 1);

	memcpy(mensaje_server, &tam_path, sizeof (int));
	memcpy(mensaje_server + sizeof (int), path, tam_path);
	memcpy(mensaje_server + sizeof(int) + tam_path, &tam_buf, sizeof (int));
	memcpy(mensaje_server + sizeof(int) + tam_path + sizeof(int), buf, tam_buf);
	memcpy(mensaje_server + 2 * sizeof(int) + tam_path + tam_buf, &offset, sizeof(int));

	mensaje_server[tam_msj] = '\0';
	char* respuesta = sac_escribir(mensaje_server);
	int estado;
	int tam;

	free (mensaje_server);

	memcpy(&estado, respuesta, sizeof(int));

	if ( estado == 1) {
		free (respuesta);

		return -1;
	}

	memcpy(&tam, respuesta + sizeof(int), sizeof(int));

	free (respuesta);

	return tam;
}


int sac_read(const char *path, char *buffer, size_t size, off_t offset,
		struct fuse_file_info *fi) {

	log_info(loggerINFO, "*** READ: SAC está...  *** \n");

	(void) fi;
	char* respuesta;
	size_t len;

	int tam_path = strlen(path);
	int tam_msj = tam_path + 3 * sizeof (int);

	int sose = size;
	int ofsote = offset;

	char* mensaje = malloc(tam_msj + 1);
	memcpy(mensaje, &tam_path, sizeof(int));
	memcpy(mensaje + sizeof(int), path, tam_path);
	memcpy(mensaje + sizeof (int) + tam_path, &sose, sizeof (int));
	memcpy(mensaje + sizeof (int) + tam_path + sizeof (int), &ofsote, sizeof (int));

	mensaje[tam_msj] = '\0';

		respuesta = sac_leer(mensaje);
		int tam_resp;

		free (mensaje);

		memcpy(&tam_resp, respuesta, sizeof(int));

		if (tam_resp == -1) {
			free (respuesta);

			return tam_resp;
		}

		else if (tam_resp == 0) {
			free (respuesta);

			return tam_resp;
		}

		memcpy(buffer, respuesta + sizeof(int), tam_resp);
		buffer[tam_resp] = '\0';


		free (respuesta);

	return tam_resp;  //número de bytes leidos
}


int sac_unlink(const char *path) {

	log_info(loggerINFO, "*** UNLINK: SAC está...  *** \n");

	int estado;
	char* resultado = sac_borrar_archivo(path);

	memcpy(&estado, resultado, sizeof(int));

	free (resultado);

	if (estado == -1)
		return -errno;

	return 0;
}

/* Para descrbir archivos y directorios */
static int sac_getattr(const char *path, struct stat *statbuf) { //Describir
	//todo enviarle un mensaje a saac server para que este envie el inodo del archivo
	//y despues llenar la structura statbuf

		char* respuesta = sac_describir(path);

		int estado;
		uint64_t fecha_creac;
		uint64_t fecha_modif;
		uint32_t tam_archivo;

		memcpy(&estado, respuesta, sizeof(int));

		if (estado == 1) {
			free (respuesta);

			return -ENOENT;
		}

		memcpy(&fecha_creac, respuesta + sizeof(int), sizeof(uint64_t));
		memcpy(&fecha_modif, respuesta + sizeof(int) + sizeof(uint64_t), sizeof(uint64_t));
		memcpy(&tam_archivo, respuesta + sizeof(int) + 2*sizeof(uint64_t), sizeof(uint32_t));

		free (respuesta);
		//printf("Tam archivo en getattr : %i\n", tam_archivo);
		memset(statbuf, 0, sizeof(struct stat));

		statbuf->st_uid = getuid(); //Usurio y grupo que montaron el file system
		statbuf->st_gid = getgid(); //Preguntar si es correcto
		statbuf->st_atime = fecha_creac;
		statbuf->st_mtime = fecha_modif;

		if (strcmp (path, ".") == 0 || strcmp (path, "..") == 0) {
			statbuf -> st_mode = S_IFDIR | 0755;
			statbuf -> st_nlink = 1;
			return 0;
		}

		else if (strcmp(path, "/") == 0) {
			statbuf->st_mode = S_IFDIR | 0755; 	// r = 4 w = 2 x = 1
			statbuf->st_nlink = 2; 	//" hardlinks: . y ..
			return 0;
		}

		else if (estado == 2) {
			statbuf -> st_mode = S_IFDIR | 0755;

			statbuf -> st_nlink = 1;

			return 0;
		}
		else if (estado == 0) {
			statbuf->st_mode = S_IFREG | 0666;
			statbuf->st_nlink = 1;
			statbuf->st_size = tam_archivo; //Predirle el tamaño a SAC-SERVER los datos del nodos
		return 0;
		}
		else {
			return -ENOENT;
		}
}


int sac_mkdir(const char *path, mode_t mode) {
	log_info(loggerINFO, "*** MKDIR: SAC está creando una carpeta *** \n");

	char *respuesta;
	int estado;

	int tam_path_modo = sizeof(int) + strlen(path) + sizeof(int);
	char* path_modo = malloc(tam_path_modo + 1);
	int tam_path = strlen(path);
	int directorio = 3;


		memcpy(path_modo, &directorio, sizeof(int));
		memcpy(path_modo + sizeof(int), &tam_path, sizeof(int));
		memcpy(path_modo + 2 * sizeof(int), path, tam_path);

		path_modo[tam_path_modo] = '\0';
		respuesta = sac_crear_nodo(path_modo);

		free (path_modo);

		memcpy(&estado, respuesta, sizeof(int));

		free (respuesta);

		if (estado != 0)
			return -errno;

		return 0;
}


int sac_rmdir(const char *path) {

	log_info(loggerINFO, "*** RMDIR: SAC está...  *** \n");

	char* respuesta = sac_borrar_directorio(path);
	int estado;

	memcpy(&estado, respuesta, sizeof(int));

	free (respuesta);

	return 0;
}


int sac_opendir(const char *path, struct fuse_file_info *fi) {
	int estado;
	int nro_nodo;

	log_info(loggerINFO, "*** OPENDIR: SAC está...  *** \n");
	char* respuesta = sac_abrir_directorio(path);

	memcpy(&estado, respuesta, sizeof(int));

	free (respuesta);

	if (estado == 1)
		return -1;
	else
		return 0;
}


int sac_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {

	log_info(loggerINFO, "\n*** READDIR: Leyendo el direcroirio %s ***\n", path);

	(void) offset;
	(void) fi;
	struct dirent *de;
	int estado;
	char* respuesta = sac_listar(path);

	memcpy(&estado, respuesta, sizeof(int));


	if (estado == -1) {
		free (respuesta);

		return -ENOENT;
	}

	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	if (estado == -3) {
		free (respuesta);

		return 0;
	}

	int tam_nombre;
	char* nombre;
	int cant_datos = sizeof(int);

	for (int i = 0; i < estado; i++) {

		memcpy(&tam_nombre, respuesta + cant_datos, sizeof(int));
		nombre = malloc(tam_nombre + 1);
		memcpy(nombre, respuesta + cant_datos + sizeof(int), tam_nombre);
		nombre[tam_nombre] = '\0';

		filler(buffer, nombre, NULL, 0);

		free (nombre);

		cant_datos += sizeof(uint8_t) + tam_nombre + 2*sizeof(int);
	}

	free (respuesta);

	return 0;
}


int sac_releasedir (const char* path, struct fuse_file_info *fi) {
	log_info(loggerINFO, "*** RELEASEDIR: SAC está...  *** \n");


	return 0;
}

int sac_truncate(const char *path, off_t newsize) {
	log_info(loggerINFO, "*** TRUNCATE: SAC está truncando  *** \n");

	int offset = newsize;

	int tam_path = strlen (path);

	char *pedido = malloc (2 * sizeof (int) + tam_path + 1);

	memcpy (pedido, &offset, sizeof (int));

	memcpy (pedido + sizeof (int), &tam_path, sizeof (int));

	memcpy (pedido + 2 * sizeof (int), path, tam_path);

	char *respuesta = sac_truncar (pedido);

	free (pedido);

	int estado;

	memcpy (&estado, respuesta, sizeof (int));

	free (respuesta);

	return estado;
}



static int sac_release(const char *path, struct fuse_file_info *fi)
{
	(void) path;
	(void) fi;
	return 0;
}


static int sac_chmod(const char *path, mode_t mode)
{

	int res;

		res = chmod(path, mode);
		if (res == -1)
			return -errno;

	return 0;
}

static int sac_rename (const char *oldpath, const char *newpath) {
	char *mensaje = malloc (2 * sizeof (int) + strlen (oldpath) + strlen (newpath) + 1);
	int tam_old_path = strlen (oldpath);
	int tam_new_path = strlen (newpath);
	char *respuesta;
	int estado;

	memcpy (mensaje, &tam_old_path, sizeof (int));

	memcpy (mensaje + sizeof (int), &tam_new_path, sizeof (int));

	memcpy (mensaje + 2 * sizeof (int), oldpath, tam_old_path);

	memcpy (mensaje + 2 * sizeof (int) + tam_old_path, newpath, tam_new_path);

	respuesta = sac_renombrar (mensaje);

	free (mensaje);

	memcpy (&estado, respuesta, sizeof (int));

	free (respuesta);

	return estado;
}

struct fuse_operations sac_cli_operations = {

	.rename = sac_rename,
	.mknod = sac_mknod,
	.utimens = sac_utimens,
	.open = sac_open,
	.access = sac_access,
	.write = sac_write,
	.read = sac_read,
	.unlink = sac_unlink,
	.getattr = sac_getattr,
	.mkdir = sac_mkdir,
	.rmdir = sac_rmdir,
	.opendir = sac_opendir,
	.readdir = sac_readdir,
	.release = sac_release,
	.releasedir = sac_releasedir,
	.truncate = sac_truncate,
	.chmod = sac_chmod,
};



int main(int argc, char* argv[]) {

	int fuse_stat;
	datos_server();
	iniciar_logs();


	pthread_mutex_init(&sem_recv, NULL);

	pthread_mutex_init(&sockete, NULL);

	/*if ((getuid() == 0) || (geteuid() == 0)) {
		log_info(loggerERROR,
				"Ejecutar sac-cli como root ocasiona agujeros de seguridad inaceptables\n");
		return 1;
	}*/

	fuse_stat = fuse_main(argc, argv, &sac_cli_operations, NULL);
	printf("\n~~~fuse_main retornó %i ~~~\n", fuse_stat);

	return fuse_stat;
}
