#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../inc/utils.h"
#include "../inc/transmission.h"
#include "../inc/config.h"

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
} arg;

long _saved_belt_capacity = -1;

int get_belt(long belt_capacity, int create) {
	int flags = create ? (O_CREAT | O_RDWR) : O_RDWR;

	int mem_id = shm_open(BELT_PATH, flags, 0666);
	if (mem_id == -1) {
		return -1;
	}

	if(ftruncate(mem_id, sizeof(Belt) + sizeof(Box) * belt_capacity) == -1) {
		return -1;
	}

	if (create) {
		_saved_belt_capacity = belt_capacity;
	}


	return mem_id;
}

void *map_belt(int mem_id) {
	void *mem_segm = mmap(NULL,
			sizeof(Belt) + sizeof(Box) * _saved_belt_capacity,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			mem_id,
			0);

	return mem_segm;
}

int unlink_belt(void *memptr) {
	return munmap(memptr, sizeof(Belt) + sizeof(Box) * _saved_belt_capacity);
}

int close_belt(int mem_id) {
	mem_id = mem_id + 1;	// just for stupid ycm not to warn me
	return shm_unlink(BELT_PATH);
}

sem_t* get_semaphore(int create) {
	int flags = create ? (O_CREAT | O_RDWR) : O_RDWR;
	sem_t* semid = sem_open(SEM_PATH, flags, 0666, 1);
	return semid;
}

int take_semaphore(sem_t * semid, int should_wait) {
	if (should_wait) {
		return sem_wait(semid);
	} else {
		return sem_trywait(semid);
	}
}

int give_semaphore(sem_t * semid) {
	return sem_post(semid);
}

int unlink_semaphore() {
	return sem_unlink(SEM_PATH);
}

int delete_semaphore(sem_t * semid) {
	return sem_close(semid);
}


Belt get_belt_from_mem(void* belt_memptr) {
	// Memory is allocated here. You have to either save
	// belt to memory all call free on belt.boxes manually
	Belt res;
	memcpy(&res, belt_memptr, sizeof(Belt));
	res.boxes = (Box *)alloc(sizeof(Box) * res.capacity);
	memcpy(res.boxes, (Box *)belt_memptr + sizeof(Belt), res.capacity * sizeof(Box)); 

	return res;
}

void save_belt_to_memory(void *belt_memptr, Belt belt) {
	Box *mem_boxes_address = belt.boxes;
	belt.boxes = ((Box *)belt_memptr + sizeof(Belt));

	memcpy(belt_memptr, &belt, sizeof(Belt)); 
	memcpy((Box *)belt_memptr + sizeof(Belt), 
			mem_boxes_address, sizeof(Box) * belt.capacity); 

	free(mem_boxes_address);
}
