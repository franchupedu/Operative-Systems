#include "commons/config.h"
#include "stdlib.h"
#include "stdio.h"

void main () {
	t_config *config = config_create ("Planificador.config");
	char *multiprocesamiento = malloc (2);
	char *quantum = malloc (2);
	char *refresh = malloc (1);
	char *puerto = malloc (1);
	char *sleep = malloc (1);
	char *ip = malloc (1);
	char salto;
	int i = 1;

	printf ("Nivel de multiprocesamiento: ");

	multiprocesamiento [0] = getchar ();

	multiprocesamiento [1] = '\0';

	salto = getchar ();

	printf ("Quantum: ");

	quantum [0] = getchar ();

	quantum [1] = '\0';

	salto = getchar ();

	printf ("Tiempo de refresh: ");

	salto = getchar ();

	while (salto != '\n') {
		refresh = realloc (refresh, i);

		refresh [i - 1] = salto;

		salto = getchar ();

		i++;
	}

	refresh [i - 1] = '\0';

	i = 1;

	printf ("Puerto: ");

	salto = getchar ();

	while (salto != '\n') {
		puerto = realloc (puerto, i);

		puerto [i - 1] = salto;

		salto = getchar ();

		i++;
	}

	puerto [i - 1] = '\0';

	i = 1;

	printf ("Tiempo de sleep: ");

	salto = getchar ();

	while (salto != '\n') {
		sleep = realloc (sleep, i);

		sleep [i - 1] = salto;

		salto = getchar ();

		i++;
	}

	sleep [i - 1] = '\0';

	i = 1;

	printf ("IP: ");

	salto = getchar ();

	while (salto != '\n') {
		ip = realloc (ip, i);

		ip [i - 1] = salto;

		salto = getchar ();

		i++;
	}

	ip [i - 1] = '\0';

	config_set_value (config, "Multiprocesamiento", multiprocesamiento);

	config_set_value (config, "Quantum", quantum);

	config_set_value (config, "Refresh", refresh);

	config_set_value (config, "Sleep", sleep);

	config_set_value (config, "Puerto", puerto);

	config_set_value (config, "IP", ip);

	config_save (config);
}
