#include "thread.h"

void thread_register(thread_t **queue, thread_t *thread) {
    thread->next = *queue;
    *queue = thread;
}
