#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "../inc/utils.h"
#include "../inc/transmission.h"
#include "../inc/config.h"
#include "../inc/environmental.h"

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
} arg;

int get_belt(long belt_capacity, int create) {
	key_t key = generate_key();
	int flags = create ? (IPC_CREAT | 0666) : 0;
	int mem_id = shmget(key,
			sizeof(Belt) + sizeof(Box) * belt_capacity, flags); 
	return mem_id;
}

void *map_belt(int mem_id) {
	void *mem_segm = shmat(mem_id, NULL, 0);
	return mem_segm;
}

int unlink_belt(void *memptr) {
	return shmdt(memptr);
}

int close_belt(int mem_id) {
	return shmctl(mem_id, IPC_RMID, NULL);
}

int get_semaphore(int create) {
	key_t key = generate_key();
	int flags = create ? (IPC_CREAT | 0666) : 0;
	int sem_id = semget(key, 1, flags);

	if (sem_id != -1 && create) {
		union semun setter;
		setter.val = 1; 	// unlocked by default
		semctl(sem_id, 0, SETVAL, setter);
	}
	return sem_id;
}

int take_semaphore(int semid, int should_wait) {
	int flags = 0;
	if (!should_wait) {
		flags = IPC_NOWAIT;
	}

	struct sembuf op;
	op.sem_num = 0;
	op.sem_op = -1; 
	op.sem_flg = flags;

	return semop(semid, &op, 1); 
}

int give_semaphore(int semid) {
	struct sembuf op;
	op.sem_num = 0;
	op.sem_op = 1; 
	op.sem_flg = 0;

	return semop(semid, &op, 1); 
}

int delete_semaphore(int semid) {
	return semctl(semid, 0, IPC_RMID, 0);
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
