#include "exec.h"


#define STDIN 0
#define STDOUT 1
#define STDERR 2

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	int i = 0;
	while (eargc > 0) {
		char *arg = eargv[i];
		char key[BUFLEN] = "";
		char value[BUFLEN] = "";
		get_environ_key(arg, key);
		int pos = block_contains(arg, '=');
		get_environ_value(arg, value, pos);
		setenv(key, value, 1);
		eargc--;
		i++;
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	return open(file, flags | O_CLOEXEC, S_IWUSR | S_IRUSR);
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:

		e = (struct execcmd *) cmd;
		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);
		// printf_debug("Command '%s' not found.\n",e->argv[0]);
		_exit(-1);
		break;

	case BACK: {
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		_exit(-1);
		break;
	}

	case REDIR: {
		r = (struct execcmd *) cmd;
		int fd;

		if (strlen(r->out_file) != 0) {
			fd = open_redir_fd(r->out_file,
			                   O_WRONLY | O_CREAT | O_TRUNC);
			dup2(fd, STDOUT);
		}

		if (strlen(r->in_file) != 0) {
			fd = open_redir_fd(r->in_file, O_RDONLY);
			if (fd == -1) {
				printf_debug("Error: %s: No such file or "
				             "directory.\n",
				             r->in_file);
				_exit(-1);
			}

			dup2(fd, STDIN);
		}

		if (strlen(r->err_file) != 0) {
			if (!(strcmp(r->err_file, "&1"))) {
				dup2(STDOUT, STDERR);
			} else {
				fd = open_redir_fd(r->err_file,
				                   O_WRONLY | O_CREAT | O_TRUNC);
				dup2(fd, STDERR);
			}
		}

		execvp(r->argv[0], r->argv);
		_exit(-1);
		break;
	}

	case PIPE: {
		p = (struct pipecmd *) cmd;

		int pipe_cmd[2];

		if (pipe(pipe_cmd) < 0) {
			printf_debug("Error en creacion de pipe\n");
			_exit(-1);
		}

		int pidleft = fork();
		if (pidleft < 0) {
			perror("Error en creacion de pipe\n");
			_exit(-1);
		}

		if (pidleft == 0) {
			close(pipe_cmd[READ]);
			dup2(pipe_cmd[WRITE], STDOUT);
			close(pipe_cmd[WRITE]);
			exec_cmd(p->leftcmd);
			_exit(0);
		}

		int pidright = fork();
		if (pidright < 0) {
			perror("Error en creacion de pipe\n");
			_exit(-1);
		}

		if (pidright == 0) {
			close(pipe_cmd[WRITE]);
			dup2(pipe_cmd[READ], STDIN);
			close(pipe_cmd[READ]);
			exec_cmd(p->rightcmd);
			_exit(0);

		} else {
			close(pipe_cmd[READ]);
			close(pipe_cmd[WRITE]);
			int rightstatus;
			waitpid(pidright, &rightstatus, 0);
		}

		if (p->rightcmd->type == EXEC) {
			int leftstatus;
			int rightstatus;
			waitpid(pidleft, &leftstatus, 0);
			waitpid(pidright, &rightstatus, 0);

			free_command(parsed_pipe);
			_exit(0);
		}


		break;
	}
	}
}
