#include "builtin.h"
#include "utils.h"
extern char promt[PRMTLEN];

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't')
		return 1;

	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (cmd[0] == 'c' && cmd[1] == 'd') {
		if (cmd[2] == END_STRING) {
			char *homepath = getenv("HOME");

			int result = chdir(homepath);
			if (result == 0) {
				promt[0] = END_STRING;
				strcat(promt, "(");
				strcat(promt, homepath);
				strcat(promt, ")");

				return 1;
			}
			return 0;
		}

		char directorio[BUFLEN] = { 0 };
		int i = 3;
		while (cmd[i] != END_STRING) {
			directorio[i - 3] = cmd[i];
			i++;
		}
		i++;
		directorio[i - 3] = END_STRING;

		int result = chdir(directorio);
		if (result == 0) {
			// Actualizar promt
			char leido[PRMTLEN];
			promt[0] = END_STRING;

			if (getcwd(leido, PRMTLEN) == NULL)
				return 0;

			strcat(promt, "(");
			strcat(promt, leido);
			strcat(promt, ")");

			return 1;
		}
	}

	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (cmd[0] == 'p' && cmd[1] == 'w' && cmd[2] == 'd') {
		if (cmd[3] != END_STRING || cmd[3] != END_LINE || cmd[3] != SPACE)
			return 0;

		char buffer[PRMTLEN];

		if (getcwd(buffer, PRMTLEN) == NULL)
			return 0;

		printf("%s\n", buffer);

		return 1;
	}

	return 0;
}
