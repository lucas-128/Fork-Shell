#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

int
main(void)
{
	srandom(time(NULL));
	printf("Hola, soy PID: %d\n", getpid());

	int fds_padre[2];  // Comunicacion padre a hijo.

	int r = pipe(fds_padre);
	if (r < 0) {
		perror("Error en creacion de pipe");
		exit(-1);
	}

	int fds_hijo[2];  // Comunicacion hijo a padre.

	r = pipe(fds_hijo);
	if (r < 0) {
		perror("Error en creacion de pipe");
		exit(-1);
	}

	printf("  - primer pipe me devuelve: [%d, %d]\n",
	       fds_padre[0],
	       fds_padre[1]);
	printf("  - segundo pipe me devuelve: [%d, %d]\n\n",
	       fds_hijo[0],
	       fds_hijo[1]);

	int i = fork();

	if (i < 0) {
		printf("Error en fork! %d\n", i);
		exit(-1);
	}

	if (i == 0) {  // Proceso hijo

		close(fds_padre[1]);  // hijo no escribe nada del lado escritura del pipe padre.
		close(fds_hijo[0]);  // hijo no lee nada del lado escritura del pipe hijo.

		printf("Donde fork me devuelve 0:\n");
		printf("  - getpid me devuelve: %d\n", getpid());
		printf("  - getppid me devuelve: %d\n", getppid());

		int recibido = 0;
		if (read(fds_padre[0], &recibido, sizeof(recibido)) == -1) {
			perror("Error en lectura\n");
			exit(-1);
		}

		printf("  - recibo valor %d vía fd=%d\n", recibido, fds_padre[0]);
		printf("  - reenvio valor en fd=%d y termino\n\n", fds_hijo[1]);

		if (write(fds_hijo[1], &recibido, sizeof(recibido)) == -1) {
			perror("Error en escritua\n");
			exit(-1);
		}

		close(fds_padre[0]);
		close(fds_hijo[1]);

		exit(0);

	} else {  // Proceso Padre

		close(fds_padre[0]);  // padre no escribe nada del lado escritura del pipe padre.
		close(fds_hijo[1]);  // padre no lee nada del lado escritura del pipe hijo.

		printf("Donde fork me devuelve %d:\n", i);
		printf("  - getpid me devuelve: %d\n", getpid());
		printf("  - getppid me devuelve: %d\n", getppid());

		int msg = random() % 100;
		printf("  - random me devuelve: %d\n", msg);

		printf("  - envio valor %d a través de fd=%d\n\n",
		       msg,
		       fds_padre[1]);  // fds_padre[1] es el pipe de escritura del padre.
		if (write(fds_padre[1], &msg, sizeof(msg)) == -1) {
			perror("Error en escritua\n");
			exit(-1);
		}

		int recibido = 0;
		if (read(fds_hijo[0], &recibido, sizeof(recibido)) == -1) {
			perror("Error en lectura\n");
			exit(-1);
		}

		wait(NULL);

		printf("Hola, de nuevo PID %d:\n", getpid());
		printf(" - recibí valor %d vía fd=%d\n", recibido, fds_hijo[0]);

		close(fds_padre[1]);
		close(fds_hijo[0]);

		exit(0);
	}

	return 0;
}
