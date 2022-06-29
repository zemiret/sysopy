#include <stdlib.h>
#include "../inc/transmission.h"
#include "../inc/utils.h"
#include "../inc/config.h"

int check_args(int argc, char **argv);
void load_box(long box_mass); 

void *belt_memptr;
int semid = -1;

int main(int argc, char **argv) {
	if (check_args(argc, argv) == -1) {
		exit(BAD_ARGS);
	}

	long box_mass = (long)strtol(argv[1], NULL, 10); 
	long cycles_count = -1;

	if (argc == 3) {
		cycles_count = strtol(argv[2], NULL, 10);
	}

	int belt_mid = get_belt(0, 0);
	if (belt_mid == -1) {
		fprintf(stderr, "Worker: %d cannot get belt\n", getpid());
		exit(MEM_ERROR);
	}

	semid = get_semaphore(0);
	printf("Semid: %d\n", semid);

	if (semid == -1) {
		fprintf(stderr, "Worker: %d cannot get semaphore\n", getpid());
		exit(SEM_ERROR);
	}

	belt_memptr = map_belt(belt_mid);

	long cur_cycle = 0;

	while (1) {
		if (cycles_count != -1 && cur_cycle >= cycles_count) {
			break;
		}

		load_box(box_mass);
		++cur_cycle;
	}

	return 0;
}

/*
 * Arguments
 * 1. box mass
 * 2. [optional] cycles count
 */
int check_args(int argc, char **argv) {
	if (argc != 2 && argc != 3) {
		perror("Bad argument count. Expected 1 or 2");
		return -1;
	}
	
	if (!is_num(argv[1]) || (argc == 3 && !is_num(argv[2]))) {
		perror("Arguments must be numeric!");
		return -1;
	}

	return 0;
}

void load_box(long box_mass) {
	int printed = 0;

	Box box;
	box.mass = box_mass;
	box.worker_id = getpid();

	while(take_semaphore(semid, 0) == -1) {
		if (!printed) {
			printf("%d is waiting for belt\n", getpid());
			printed = 1;
		}
	}
	// here we should have semaphore
	Belt belt = get_belt_from_mem(belt_memptr);

	long next_box_idx = (belt.current_box + 1) % belt.capacity; 

	// if conditions are met (mass, capacity, etc, load box)
	if (belt.boxes[next_box_idx].mass == -1 &&	
			belt.current_mass + box.mass <= belt.mass_capacity) {

		box.timestamp = get_timestamp();
		belt.boxes[next_box_idx] = box;

		save_belt_to_memory(belt_memptr, belt);
		give_semaphore(semid);

    	printf("%d loaded box with mass: %ld and timestamp %lld\n",
    			getpid(),
    			box.mass,
    			box.timestamp);

	} else {
		free(belt.boxes);
		give_semaphore(semid);
	}
}
