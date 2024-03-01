#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

void filtrar(int fd_lectura);

void
filtrar(int fd_lectura)
{
	int primero = 0;
	if (read(fd_lectura, &primero, sizeof(primero)) != 0) {
		printf("primo %d\n", primero);

		int pipe_hermanos[2];

		int r = pipe(pipe_hermanos);
		if (r < 0) {
			perror("Error en creacion de pipe\n");
			exit(-1);
		}

		int i = fork();

		if (i < 0) {
			printf("Error en fork! %d\n", i);
			exit(-1);
		}

		if (i != 0) {
			close(fd_lectura);
			close(pipe_hermanos[1]);
			wait(NULL);
			filtrar(pipe_hermanos[0]);
			exit(0);

		} else {
			close(pipe_hermanos[0]);
			int leido = 0;
			while (read(fd_lectura, &leido, sizeof(leido))) {
				if (leido % primero != 0) {
					ssize_t escritos = write(pipe_hermanos[1],
					                         &leido,
					                         sizeof(leido));  // Escribo en pipe los numeros no filtrados
					if (escritos == -1) {
						perror("Error en write!\n");
						exit(-1);
					}
				}
			}
			close(pipe_hermanos[1]);
			close(fd_lectura);
			exit(0);
		}
	}

	// Cuando el pipe esta vacio, cierro y finalizo el ciclo.
	close(fd_lectura);
	exit(0);
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		perror("Cantidad de parametros erroneas\n");
		exit(-1);
	}

	int pipe_padre[2];

	int r = pipe(pipe_padre);
	if (r < 0) {
		perror("Error en creacion de pipe\n");
		exit(-1);
	}

	int i = fork();

	if (i < 0) {
		printf("Error en fork! %d\n", i);
		exit(-1);
	}

	if (i != 0) {
		close(pipe_padre[1]);
		wait(NULL);
		// Inicia ciclo de filtracion
		filtrar(pipe_padre[0]);
		close(pipe_padre[0]);

	} else {
		close(pipe_padre[0]);
		int n = atoi(argv[1]);
		// Escritura de secuencia en pipe
		for (int i = 2; i <= n; i++) {
			ssize_t escritos = write(pipe_padre[1], &i, sizeof(i));
			if (escritos == -1) {
				perror("Error en write!\n");
				exit(-1);
			}
		}
		close(pipe_padre[1]);
	}
	exit(0);
}
