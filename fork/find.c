#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void find(char *nombre,
          DIR *directory,
          char *(*comparar)(const char *, const char *),
          char buffer[]);

void
find(char *nombre,
     DIR *directory,
     char *(*comparar)(const char *, const char *),
     char buffer[])
{
	struct dirent *entry;
	while (((entry) = readdir(directory))) {
		if (entry->d_type == DT_DIR && (strcmp(entry->d_name, ".")) &&
		    (strcmp(entry->d_name, ".."))) {
			if ((comparar(entry->d_name, nombre))) {
				printf("%s", buffer);
				printf("%s\n", entry->d_name);
			}

			int fd_directorio = dirfd(directory);
			int fd_siguiente =
			        openat(fd_directorio, entry->d_name, O_DIRECTORY);
			DIR *dir_siguiente = fdopendir(fd_siguiente);

			char buffer_aux[PATH_MAX];
			strcpy(buffer_aux, buffer);

			strcat(buffer, entry->d_name);
			strcat(buffer, "/");

			find(nombre, dir_siguiente, comparar, buffer);
			closedir(dir_siguiente);

			strcpy(buffer, buffer_aux);
		}

		if (entry->d_type == DT_REG) {
			if (comparar(entry->d_name, nombre) != NULL) {
				printf("%s", buffer);
				printf("%s\n", entry->d_name);
			}
		}
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		perror("Cantidad de parametros erroneas\n");
		exit(-1);
	}

	DIR *directory = opendir(".");
	if (directory == NULL) {
		perror("error con opendir");
		exit(-1);
	}

	char buffer[PATH_MAX];
	strcpy(buffer, "");

	char *nombre;
	if (!strcmp(argv[1], "-i")) {
		nombre = argv[2];
		find(nombre, directory, strcasestr, buffer);
	} else {
		nombre = argv[1];
		find(nombre, directory, strstr, buffer);
	}

	closedir(directory);

	return 0;
}
