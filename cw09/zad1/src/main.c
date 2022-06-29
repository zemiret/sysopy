#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "../inc/config.h"
#include "../inc/utils.h"


int check_args(int argc, char **argv);

void* passenger_routine(void * args);
void* cart_routine(void * args);

typedef struct {
	int id;
	int cart_id;	// if -1 then he is not on the cart
} Passenger;

typedef struct {
	int id;
	int capacity;
	int rides_left;
	int people_count;
	int is_unloading;
	int is_closed;
	int is_riding;
} Cart;

typedef struct {
	Cart* carts;
	int current_cart;	// If -1, then no carts on the platform
	Passenger* passengers;
} Platform;

// Shared vars
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


Platform *platform;


int main(int argc, char **argv) {
	if (check_args(argc, argv) == -1) {
		exit(BAD_ARGS);
	}

	int passengers_count = (int)strtol(argv[1], NULL, 10);
	int carts_count = (int)strtol(argv[2], NULL, 10);
	int carts_capacity = (int)strtol(argv[3], NULL, 10);
	int rides_count = (int)strtol(argv[4], NULL, 10);

	// Init platform
	platform = (Platform *)alloc(sizeof(Platform));
	platform->carts = (Cart *)alloc(sizeof(Cart) * carts_count);
	platform->passengers = (Passenger *)alloc(sizeof(Passenger) * passengers_count);

	for (int i = 0; i < carts_count; ++i) {
		Cart c;
		c.id = i;
		c.capacity = carts_capacity;
		c.rides_left = rides_count;
		c.people_count = 0;
		c.is_unloading = 0;
		c.is_closed = 1;
		c.is_riding = 0;
		platform->carts[i] = c;
	}

	for (int i = 0; i < passengers_count; ++i) {
		Passenger p;
		p.id = i;
		p.cart_id = -1;
		platform->passengers[i] = p;
	}

	platform->current_cart = -1;

	// Let the roller coaster begin
	printf("--- START ---\nPassengers: %d\nCarts: %d\nCapacity: %d\n\n\n",
			passengers_count,
			carts_count,
			carts_capacity);

	pthread_t *cart_tids = (pthread_t*)alloc(carts_count * sizeof(pthread_t));
	pthread_t *passenger_tids = (pthread_t*)alloc(passengers_count * sizeof(pthread_t));

	// Create threads
	for (int i = 0; i < passengers_count; ++i) {
		if (pthread_create(&passenger_tids[i], NULL, passenger_routine, &platform->passengers[i]) == -1) {
			fprintf(stderr, "Errror creating passenger thread %d\n", i);
		}
	}

	for (int i = 0; i < carts_count; ++i) {
		if (pthread_create(&cart_tids[i], NULL, cart_routine,
					&platform->carts[i]) == -1) {
			fprintf(stderr, "Errror creating cart thread %d\n", i);
		}
	}


	for (int i = 0; i < carts_count; ++i) {
		if(pthread_join(cart_tids[i], NULL) != 0) {
			fprintf(stderr, "Error waiting for thread: %lu\n", cart_tids[i]);
		}
	}
	
	for(int i = 0; i < passengers_count; ++i) {
		if(pthread_cancel(passenger_tids[i]) != 0) {
			fprintf(stderr, "Error killing thread: %lu\n", passenger_tids[i]);
		}
		printf("%llu: [CLIENT: %d] I'm dying!\n", get_timestamp(), i);
	}

	free(platform->carts);
	free(platform->passengers);
	free(platform);
	free(cart_tids);
	return 0;
}

/**
 * Arguments:
 * 1. Passagers count 
 * 2. Carts count 
 * 3. Cart capacity 
 * 4. Number of rides 
 **/ int check_args(int argc, char **argv) {
	if (argc != 5) {
		perror("Wrong arguments number. Expected 4\n");
		return -1;
	}

	for (int i = 1; i <= 4; ++i) {
		if (!is_num(argv[i])) {
			perror("All arguments must be numeric\n");
			return -1;
		}
	}

	return 0;
}

void* passenger_routine(void * args) {
	Passenger* passenger = (Passenger *)args;

	while(1) {
		pthread_mutex_lock(&mutex);
		int cur_cart = platform->current_cart;
		while(cur_cart == -1 || 
				platform->carts[cur_cart].is_unloading ||
				platform->carts[cur_cart].capacity == platform->carts[cur_cart].people_count || 
				platform->carts[cur_cart].is_closed){

			pthread_cond_wait(&cond, &mutex);
		    cur_cart = platform->current_cart;
		}

		++platform->carts[cur_cart].people_count;

		printf("%llu: [CLIENT: %d]: Getting in %d! In there: %d/%d\n",
				get_timestamp(),
				passenger->id,
				cur_cart,
				platform->carts[cur_cart].people_count,
				platform->carts[cur_cart].capacity);

		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);

		pthread_mutex_lock(&mutex);

		while (!platform->carts[cur_cart].is_closed &&
				!platform->carts[cur_cart].is_riding &&
				!platform->carts[cur_cart].is_unloading) {
			pthread_cond_wait(&cond, &mutex);
		}
		if (!platform->carts[cur_cart].is_riding) {
			printf("%llu: [CLIENT: %d]: Starting %d! In there: %d/%d\n",
					get_timestamp(),
					passenger->id,
					cur_cart,
					platform->carts[cur_cart].people_count,
					platform->carts[cur_cart].capacity);

			platform->carts[cur_cart].is_riding = 1;
			pthread_cond_broadcast(&cond);
		}

		while (platform->carts[cur_cart].is_unloading == 0) {
			pthread_cond_wait(&cond, &mutex);
		}

		--platform->carts[cur_cart].people_count;
		
		printf("%llu: [CLIENT: %d]: Getting out of %d! In there: %d/%d\n",
				get_timestamp(),
				passenger->id,
				cur_cart,
				platform->carts[cur_cart].people_count,
				platform->carts[cur_cart].capacity);

		if(platform->carts[cur_cart].people_count == 0) {
			pthread_cond_broadcast(&cond);
		}
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

void* cart_routine(void * args) {
	Cart *cart = (Cart *)args;

	while (cart->rides_left > 0) {
	    pthread_mutex_lock(&mutex);
	    while (platform->current_cart != -1) {
	    	pthread_cond_wait(&cond, &mutex);
	    }

	    platform->current_cart = cart->id; 
		printf("%llu: [CART: %d] Opening a door wide!\n", get_timestamp(), cart->id);
		cart->is_closed = 0;
		if (cart->people_count > 0) {
			// load out
			cart->is_unloading = 1;

			pthread_cond_broadcast(&cond);

			while(cart->people_count > 0) {
				pthread_cond_wait(&cond, &mutex);
			}

			cart->is_unloading = 0;
		}
		pthread_cond_broadcast(&cond);

		while(cart->people_count != cart->capacity) {
			pthread_cond_wait(&cond, &mutex);
		}

		printf("%llu: [CART: %d] Closing a door shut!\n", get_timestamp(), cart->id);
		cart->is_closed = 1;
		pthread_cond_broadcast(&cond);

		while(!cart->is_riding) {
			pthread_cond_wait(&cond, &mutex);
		}

		printf("%llu: [CART: %d] Starting a ride!\n", get_timestamp(), cart->id);
		platform->current_cart = -1;
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&mutex);

		struct timespec sleeptime;
		sleeptime.tv_nsec = 20000000;

		nanosleep(&sleeptime, NULL);
		printf("%llu: [CART: %d] Finished a ride!\n", get_timestamp(), cart->id);

		pthread_mutex_lock(&mutex);
		cart->is_riding = 0;
		--cart->rides_left;
		pthread_mutex_unlock(&mutex);

	}

	printf("%llu: [CART: %d] I'm dying!\n", get_timestamp(), cart->id);

	return NULL;
}
