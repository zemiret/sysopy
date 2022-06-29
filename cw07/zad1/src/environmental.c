#include "../inc/environmental.h"
#include "../inc/config.h"
#include <sys/ipc.h>
#include <stdlib.h>


key_t generate_key() {
	return ftok(getenv("HOME"), PROJ_ID);
}
