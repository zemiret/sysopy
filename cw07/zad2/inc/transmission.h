#ifndef TRANSMISSION_H
#define TRANSMISSION_H

#include <unistd.h>
#include <semaphore.h>
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

sem_t* get_semaphore(int create);
int take_semaphore(sem_t* semid, int should_wait);
int give_semaphore(sem_t* semid);
int delete_semaphore(sem_t* semid);
int unlink_semaphore();

Belt get_belt_from_mem(void *belt_memptr);
void save_belt_to_memory(void *belt_memptr, Belt belt);


#endif

