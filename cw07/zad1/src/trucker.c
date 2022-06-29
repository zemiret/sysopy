#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "../inc/config.h"
#include "../inc/utils.h"
#include "../inc/transmission.h"

typedef struct {
	long mass_capacity;
	long cur_mass;
} Truck;

int check_args(int argc, char **argv);

void register_signal_handlers();
void on_sigint(int signum);

void create_belt(long capacity, long mass_capacity);

Box peek_box();
void take_box(Truck* truck);
void unload_truck(Truck* truck);

int belt_mid = -1;
void *belt_memptr = NULL; 
int semid = -1;

Truck truck;

int main(int argc, char **argv) {
	if (check_args(argc, argv) == -1) {
		exit(BAD_ARGS);
	}

	long trucker_mass_capacity = strtol(argv[1], NULL, 10);
	long belt_capacity = strtol(argv[2], NULL, 10);
	long belt_mass_capacity = strtol(argv[3], NULL, 10);

	belt_mid = get_belt(belt_capacity, 1);
	belt_memptr = map_belt(belt_mid); 

	semid = get_semaphore(1);
	printf("Semid: %d\n", semid);

	if (semid == -1) {
		perror("Cannot create semaphore\n");
		exit(SEM_ERROR);
	}

	if (belt_memptr == (void *)-1) {
		perror("Cannot create shared memory segment\n");
		exit(MEM_ERROR);
	}

	create_belt(belt_capacity, belt_mass_capacity);
	truck.cur_mass = 0;
	truck.mass_capacity = trucker_mass_capacity;

	register_signal_handlers();

	int printed = 0;

	while(1) {
		Box next_box = peek_box();

		if (next_box.mass == -1) {
			if (printed == 0) {
				printf("No next box on belt. Waiting\n");
				printed = 1;
			}
			continue;
		}
		printed = 0;

		if (next_box.mass + truck.cur_mass > truck.mass_capacity) {
			unload_truck(&truck);
		} else {
			take_box(&truck);
		}
	}

	return 0;
}

/*
 * Arguments
 * 1. trucker mass capacity
 * 2. belt capacity
 * 3. belt mass capacity
 */
int check_args(int argc, char **argv) {
	if (argc != 4) {
		perror("Bad argument count. Expected 3");
		return -1;
	}
	
	if (!is_num(argv[1]) || 
			!is_num(argv[2]) ||
			!is_num(argv[3])) {
		perror("Arguments must be numeric!");
		return -1;
	}

	return 0;
}

void register_signal_handlers() {
	struct sigaction act;
	act.sa_handler = on_sigint;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGINT, &act, NULL);
}

void on_sigint(int signum) {
	perror("Cleaning stuff\n");

	signum = signum - 1;	// just for stupid YCM
	if (unlink_belt(belt_memptr) == -1) {
		perror("Cannot unlink belt\n");
		exit(MEM_ERROR);
	}
	if (close_belt(belt_mid) == -1) {
		perror("Cannot unlink belt\n");
		exit(MEM_ERROR);
	}

	if (delete_semaphore(semid) == -1) {
		perror("Cannot delete semaphore\n");
		exit(SEM_ERROR);
	}

	exit(0);
}


void create_belt(long capacity, long mass_capacity) {
	Belt belt;
	belt.capacity = capacity;
	belt.mass_capacity = mass_capacity;
	belt.current_box = 0;
	belt.current_mass = 0;

	belt.boxes = (Box *)alloc(sizeof(Box) * capacity); 
	for (int i = 0; i < capacity; ++i) {
		Box nullbox;
		nullbox.mass = -1;
		nullbox.worker_id = -1;
		belt.boxes[i] = nullbox;
	}

	save_belt_to_memory(belt_memptr, belt);
}

Box peek_box() {
	take_semaphore(semid, 1);
	Belt belt = get_belt_from_mem(belt_memptr);
	give_semaphore(semid);

	long next_box_idx = (belt.current_box + 1) % belt.capacity;
	Box res = belt.boxes[next_box_idx];

	free(belt.boxes);

	return res;
}

void take_box(Truck* truck) {
	take_semaphore(semid, 1);

	Belt belt = get_belt_from_mem(belt_memptr);
	
	// load box on truck
	long next_box_idx = (belt.current_box + 1) % belt.capacity;
	Box next_box = belt.boxes[next_box_idx];

	printf("Loading. Mass: %ld, worker: %d, timediff: %llu us. Current mass: %ld\n",
			next_box.mass, next_box.worker_id,
			get_timestamp() - next_box.timestamp,
			truck->cur_mass);

	truck->cur_mass += next_box.mass; 

	// remove box from belt
	belt.boxes[belt.current_box].mass = -1;
	belt.current_box = next_box_idx;

	save_belt_to_memory(belt_memptr, belt);

	give_semaphore(semid);
}

void unload_truck(Truck* truck) {
	take_semaphore(semid, 1);

	printf("Unloading truck\n");
	truck->cur_mass = 0;
	printf("Truck unloaded\n");

	give_semaphore(semid);
}

