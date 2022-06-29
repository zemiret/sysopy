#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libblock.h"

char **blocks = NULL;
int SIZE; 
char *dir_name = NULL;
char *tmp_file_name = NULL;
char *file_name = NULL;

void free_blocks();
void free_search_dirs();

void create_blocks(const int BLK_SIZE) {
	free_blocks();

	SIZE = BLK_SIZE;
	blocks = (char**)calloc(SIZE, sizeof(char *));
	for(int i = 0; i < SIZE; ++i) {
		blocks[i] = NULL;
	}
}

void set_search_dir_and_file(
		const char* dir_name_set,
		const char* tmp_file_name_set,
		const char* file_name_set
		) {

	free_search_dirs();

	// TODO: Seems like one source of error is here, some missetting of ptr from argv?
	dir_name = (char *)calloc(strlen(dir_name_set), sizeof(char));
	tmp_file_name = (char *)calloc(strlen(tmp_file_name_set), sizeof(char));
	file_name = (char *)calloc(strlen(file_name_set), sizeof(char));

	strcpy(dir_name, dir_name_set);
	strcpy(tmp_file_name, tmp_file_name_set);
	strcpy(file_name, file_name_set); 
}

void search_and_tmp_save() {
	char *command = (char *)calloc(
			strlen("find ") + 
			strlen(dir_name) + 
			strlen(" -name ") + 
			strlen(file_name) + 
			strlen(" > ") + 
			strlen(tmp_file_name),
			sizeof(char)
			);

	strcat(command, "find ");
	strcat(command, dir_name);
	strcat(command, " -name ");
	strcat(command, file_name);
	strcat(command, " > ");
	strcat(command, tmp_file_name);

	system(command);

	free(command);
}

int create_block_from_tmp_file() {
	int index = 0;
	while (index < SIZE && blocks[index] != NULL) {
		++index;
	}
	if (index == SIZE) {
		fprintf(stderr, "%s\n", "There are no more blocks, reallocate\n");
		return -1;
	}

	FILE* fp = fopen(tmp_file_name, "r");
	if (fp == NULL) {
		fprintf(stderr, "%s - %s\n", "Couldn't open file for reading", tmp_file_name);
		return -1;
	}

	// get the block length
	fseek(fp, 0, SEEK_END);
	int blksize = ftell(fp); 

	// allocate
	blocks[index] = (char *)calloc(blksize, sizeof(char));

	// read the block
	fseek(fp, 0, SEEK_SET);
	fread(blocks[index], sizeof(char), blksize, fp);

	fclose(fp);
	return index;
}

void delete_block(int index) {
	if (blocks[index] != NULL) {
		free(blocks[index]);
		blocks[index] = NULL;
	}
}

void free_mem() {
	free_blocks();
	free_search_dirs();
}

/*
 * "private" functions
 */
void free_blocks() {
	for(int i = 0; i < SIZE; ++i) {
		delete_block(i);
	}
	if (blocks != NULL) {
		free(blocks);
		blocks = NULL;
	}
}

void free_search_dirs() {
	if (dir_name != NULL) {
		free(dir_name);
		dir_name = NULL;
	}
	if (tmp_file_name != NULL) {
		free(tmp_file_name);
		tmp_file_name = NULL;
	}
	if (file_name != NULL) {
		free(file_name);
		file_name = NULL;
	}
}

