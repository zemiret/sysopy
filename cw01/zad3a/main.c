#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include "libblock.h"


struct clock {
	clock_t st_time;
	clock_t en_time;
	struct tms st_cpu;
	struct tms en_cpu;
};

void start_clock(struct clock *clk) {
    clk->st_time = times(&(clk->st_cpu));
}

void end_clock(struct clock *clk, const char *msg) {
    clk->en_time = times(&(clk->en_cpu));

	clk->en_time = (long double)clk->en_time;
	clk->st_time = (long double)clk->st_time;
    long double tick_per_sec = (long double)sysconf(_SC_CLK_TCK);
	
	printf("--- %s ---\n", msg);
    printf("Real Time: %Lfms, User Time %Lfms, System Time %Lfms\n",
        ((clk->en_time - clk->st_time) * 1000.0 / tick_per_sec),
        ((clk->en_cpu.tms_utime - clk->st_cpu.tms_utime) * 1000 / tick_per_sec),
        ((clk->en_cpu.tms_stime - clk->st_cpu.tms_stime) * 1000 / tick_per_sec));
}

char* get_next_arg(int argc, char **argv, int cur) {
	if (cur >= argc) {
		fprintf(stderr, "%s", "Invalid command format");
		exit(1);
	}
	return argv[cur];
}

void execute_commands(int argc, char **argv) {
	int i = 2;
	while (i < argc) {
		char *cmd = get_next_arg(argc, argv, i);
		++i;

		printf("cmd: %s\n", cmd);

		if (strcmp(cmd, "search_directory") == 0) {
			char *dir_name = get_next_arg(argc, argv, i);
			++i;
			char *file_name = get_next_arg(argc, argv, i);
			++i;
			char *tmp_file_name = get_next_arg(argc, argv, i);
			++i;

			set_search_dir_and_file(dir_name, tmp_file_name, file_name);
			search_and_tmp_save();
		} else if(strcmp(cmd, "create_block") == 0) {
			int res = create_block_from_tmp_file();
			if (res == -1) {
				exit(1);
			}
		} else if(strcmp(cmd, "remove_block") == 0) {
			char *index_str = get_next_arg(argc, argv, i);
			++i;
			int index = strtol(index_str, NULL, 10);
			delete_block(index);
		} else {
			fprintf(stderr, "%s: %s", "Invalid operation", argv[i - 1]);
			exit(1);
		}
	}
}

int check_params_and_get_size(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "%s", "No argument given.");
		exit(1);
	}

	const int SIZE = (int)strtol(argv[1], NULL, 10);
	if (SIZE == 0) {
		fprintf(stderr, "%s", "Supply a valid size as first argument");
		exit(1);
	}

	return SIZE;
}

void benchmark_searches() {
	// small
	struct clock *clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);
	set_search_dir_and_file("search_dir/small", "/tmp/res", "file1.txt");
	search_and_tmp_save();
	end_clock(clk, "Small directory search");
	free(clk);
	// medium
	clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);
	set_search_dir_and_file("search_dir/medium", "/tmp/res", "file1.txt");
	search_and_tmp_save();
	end_clock(clk, "Medium directory search");
	free(clk);
	// big
	clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);
	set_search_dir_and_file("search_dir/big", "/tmp/res", "file1.txt");
	search_and_tmp_save();
	end_clock(clk, "Big directory search");
	free(clk);
}

void benchmark_saving() {
	// small
	set_search_dir_and_file("search_dir/small", "/tmp/res", "file1.txt");
	search_and_tmp_save();

	struct clock *clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);
	create_block_from_tmp_file();
	end_clock(clk, "Small block save");
	free(clk);
	// medium
	set_search_dir_and_file("search_dir/medium", "/tmp/res", "file1.txt");
	search_and_tmp_save();

	clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);
	create_block_from_tmp_file();
	end_clock(clk, "Medium block save");
	free(clk);
	// big
	set_search_dir_and_file("search_dir/big", "/tmp/res", "file1.txt");
	search_and_tmp_save();

	clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);
	create_block_from_tmp_file();
	end_clock(clk, "Big block save");
	free(clk);
}

void benchmark_deleting() {
	// small
	set_search_dir_and_file("search_dir/small", "/tmp/res", "file1.txt");
	search_and_tmp_save();
	int smIndex = create_block_from_tmp_file();

	struct clock *clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);
	delete_block(smIndex);
	end_clock(clk, "Small block delete");
	free(clk);
	// medium
	set_search_dir_and_file("search_dir/medium", "/tmp/res", "file1.txt");
	search_and_tmp_save();
	int mIndex = create_block_from_tmp_file();

	clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);
	delete_block(mIndex);
	end_clock(clk, "Medium block delete");
	free(clk);
	// big
	set_search_dir_and_file("search_dir/big", "/tmp/res", "file1.txt");
	search_and_tmp_save();
	int bIndex = create_block_from_tmp_file();

	clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);
	delete_block(bIndex);
	end_clock(clk, "Big block delete");
	free(clk);
}

void benchmark_creating_and_deleting() {
	// small
	set_search_dir_and_file("search_dir/small", "/tmp/res", "file1.txt");
	search_and_tmp_save();

	struct clock *clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);

	for (int i = 0; i < 1000; ++i) {
		int smIndex = create_block_from_tmp_file();
		delete_block(smIndex);
	}

	end_clock(clk, "1000x small block create and delete");
	free(clk);
	// medium
	set_search_dir_and_file("search_dir/medium", "/tmp/res", "file1.txt");
	search_and_tmp_save();

	clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);

	for(int i = 0; i < 1000; ++i) {
		int mIndex = create_block_from_tmp_file();
		delete_block(mIndex);
	}

	end_clock(clk, "1000x medium block create and delete");
	free(clk);
	// big
	set_search_dir_and_file("search_dir/big", "/tmp/res", "file1.txt");
	search_and_tmp_save();

	clk = (struct clock *)malloc(sizeof(struct clock));
	start_clock(clk);

	for(int i = 0; i < 1000; ++i) {
		int bIndex = create_block_from_tmp_file();
		delete_block(bIndex);
	}

	end_clock(clk, "1000x big block create and delete");
	free(clk);
}


int main(int argc, char** argv) {
	const int SIZE = check_params_and_get_size(argc, argv);
	create_blocks(SIZE);

	if (argc >= 3) {
		if(strcmp(argv[2], "benchmark") == 0) {
        	benchmark_searches();
        	benchmark_saving();
        	benchmark_deleting();
        	benchmark_creating_and_deleting();
		} else {
    		struct clock *clk = (struct clock *)malloc(sizeof(struct clock)); 
			start_clock(clk);
    		execute_commands(argc, argv);
    		end_clock(clk, "Command execution");
    		free(clk);
		}
	}

	// free the memory
	free_mem();

	return 0;
}
