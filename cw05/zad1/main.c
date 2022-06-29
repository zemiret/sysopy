#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils/utils.h"
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


void check_args(int argc, char **argv);

char **read_commands(char *filename, int *count);
char **split_command(char *commands, int *count, const char *delim);
void free_commands(char **commands, int count);

void execute(char *command_line);
void exec_command(char *cmd);

int main(int argc, char **argv) {
	check_args(argc, argv);

	int cmd_count;
	char **commands = read_commands(argv[1], &cmd_count);

	for (int i = 0; i < cmd_count; ++i) {
		if (!is_empty(commands[i])) {
			execute(commands[i]);
		}
	}

	// for each line in file
	free_commands(commands, cmd_count);
}


void check_args(int argc, char **argv) {
	if (argc != 2) {
		perror("Incorrct argument number. Expected 1\n");	
		exit(-1);
	}

	if (!fileexists(argv[1])) {
		perror("Incorrct argument 1. File does not exist.\n");	
		exit(-1);
	}
}

char **read_commands(char *filename, int *count) {
	FILE* fh = fopen(filename, "r");
	assert_file_ok(fh, filename);

	int line_count = get_line_count(fh);

	assert_file_ok(fh, filename);

	char **commands = (char **)alloc(line_count * sizeof(char *));
	for (int i = 0; i < line_count; ++i) {
		commands[i] = NULL;
	}
	
	char *line = NULL;
	size_t n = 0;
	int i = 0;
	int prev_errno = errno;

	while (getline(&line, &n, fh) != -1) {
		commands[i] = (char *)alloc(strlen(line) + 1);
		strcpy(commands[i], line);

		++i;
		free(line);
		line = NULL;
	}
	free(line);

	if (errno != prev_errno) {
		fprintf(stderr, "Error while reading file %s\n", filename);
		exit(-1);
	}

	*count = line_count;

	fclose(fh);
	return commands;
}

char **split_command(char *command, int *count, const char *delim) {
	*count = count_in_string(command, delim) + 1;
	char **res = (char **)alloc(*count * sizeof(char *));
	for (int i = 0; i < *count; ++i) {
		res[i] = NULL;
	}

    char* single_cmd = strtok(command, delim);
	int i = 0;
    while (single_cmd) {
		char *trimmed = NULL;
		int len = trim_string(&trimmed, single_cmd);

		res[i] = (char *)alloc(len + 1); 
		strcpy(res[i], trimmed);

		free(trimmed);

        single_cmd = strtok(NULL, "|");
		++i;
    }

	return res;
}


void free_commands(char **commands, int count) {
	for (int i = 0; i < count; ++i) {
		if (commands[i] != NULL) {
			free(commands[i]);
		}
	}
	free(commands);
}

void execute(char *command_line) {
	// 1. read all commands
	// 2. for each command:
	// - create a pipe
	// - connect STDIN to read of prev command (if such exists)
	// - connect STDOUT to write of current pipe 
	// - execute? (maybe this should happen after all pipes are set?)
	
	int command_count;
	char **commands = split_command(command_line, &command_count, "|"); 

	int prev_pipe[2] = {-1, -1};
	int prev_prev_pipe[2] = {-1, -1};

	for (int i = 0; i < command_count; ++i) {
		// - create a pipe
		int cur_pipe[2];
		if(pipe(cur_pipe) == -1) {
			perror("Error creating pipe");	
			exit(-1);
		}

		int pid = fork();
		if (pid == -1) {
			perror("Error forking");
			exit(-1);
		}

		if(pid == 0) {
			// child
			// - connect STDOUT to write of current pipe 
			if (i != command_count - 1) {
				dup2(cur_pipe[1], STDOUT_FILENO);
			}

			// - connect STDIN to read of prev command (if such exists)
			if (prev_pipe[0] != -1) { // prev pipe reading end 
				dup2(prev_pipe[0], STDIN_FILENO);
			}

			close(cur_pipe[0]);
			close(cur_pipe[1]);
			if (prev_pipe[0] != -1) { close(prev_pipe[0]); }
			if (prev_pipe[1] != -1) { close(prev_pipe[1]); }


			// - execute? (maybe this should happen after all pipes are set?)
			exec_command(commands[i]);
			perror("Bad exec");
			exit(-1);
		} else {
			prev_prev_pipe[0] = prev_pipe[0];
			prev_prev_pipe[1] = prev_pipe[1];

			prev_pipe[0] = cur_pipe[0];
			prev_pipe[1] = cur_pipe[1];

			if (prev_prev_pipe[0] != -1) { close(prev_prev_pipe[0]); };
			if (prev_prev_pipe[1] != -1) { close(prev_prev_pipe[1]); };
		}
	}

	free_commands(commands, command_count);
}

void exec_command(char *cmd) {
	int count;
	char *trimmed = NULL;
	trim_string(&trimmed, cmd);
	char **command_and_args = split_command(trimmed, &count, " ");
	free(trimmed);

	char **null_terminated = (char **)alloc((count + 1) * sizeof(char *));
	for (int i = 0; i < count + 1; ++i) {
		null_terminated[i] = NULL;
	}

	for (int i = 0; i < count; ++i) {
		null_terminated[i] = command_and_args[i];
	}

//	for (int i = 0; i < count + 1; ++i) {
//		fprintf(stderr, "%s ", null_terminated[i]);
//	}
//	fprintf(stderr, "\n");

	execvp(null_terminated[0], null_terminated);
	// should I free? it doesn't go here since it's exec
	free_commands(command_and_args, count);
	free(null_terminated);
}
