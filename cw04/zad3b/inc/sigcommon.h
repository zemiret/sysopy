#ifndef SIGCOMMON_H
#define SIGCOMMON_H

#include <signal.h>

#define MASK_SET_ERR -199

#define KILL "KILL"
#define SIGQUEUE "SIGQUEUE"
#define SIG_RT "SIGRT"

void send_signal(int proc_pid, int signo, const char *type);
void register_signal_handlers(void (*handle_signal)(int, siginfo_t *, void *));

#endif
