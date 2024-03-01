#ifndef NARGS
#define NARGS 4
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		perror("Cantidad de parametros erroneas\n");
		exit(-1);
	}

	char buffer[50];
	char *b = buffer;
	size_t bufsize = 50;
	size_t chars_leidos;

	char *lista[NARGS + 2];
	lista[0] = argv[1];

	int lineas_leidas = 0;

	while ((chars_leidos = (getline(&b, &bufsize, stdin))) != (size_t) -1) {
		if ((buffer)[chars_leidos - 1] == '\n') {
			(buffer)[chars_leidos - 1] = '\0';
		}

		lineas_leidas++;

		lista[lineas_leidas] = malloc(strlen(b) + 1);
		strcpy(lista[lineas_leidas], b);

		if (lineas_leidas == NARGS) {
			lista[NARGS + 1] = NULL;

			int i = fork();

			if (i < 0) {
				printf("Error en fork! %d\n", i);
				exit(-1);
			}

			if (i == 0) {
				execvp(lista[0], lista);
				perror("Error en execvp!\n");
				exit(-1);

			} else {
				int status;
				if (wait(&status) == -1) {
					perror("Falla en wait");
					exit(-1);
				}

				for (int i = 1; i < NARGS + 1; i++) {
					free(lista[i]);
				}
			}
			lineas_leidas = 0;
		}
	}

	// Ultimo llamado a execvp para las lineas que quedaron sin empaquetar
	if (lineas_leidas > 0) {
		lista[lineas_leidas + 1] = NULL;
		execvp(lista[0], lista);
		perror("Error en execvp!\n");
		exit(-1);
	}

	return 0;
}