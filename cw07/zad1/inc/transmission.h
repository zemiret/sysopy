#ifndef TRANSMISSION_H
#define TRANSMISSION_H

#include <unistd.h>
#include <sys/types.h>

typedef struct {
	long mass;
	pid_t worker_id;	
	unsigned long long timestamp;
} Box;

typedef struct {
	long capacity;
	long mass_capacity;
	long current_mass;

	long current_box;
	Box *boxes;
} Belt;


int get_belt(long belt_capacity, int create);
void *map_belt(int mem_id);

int unlink_belt(void *memptr);
int close_belt(int mem_id);

int get_semaphore(int create);
int take_semaphore(int semid, int should_wait);
int give_semaphore(int semid);
int delete_semaphore(int semid);

Belt get_belt_from_mem(void *belt_memptr);
void save_belt_to_memory(void *belt_memptr, Belt belt);


#endif

