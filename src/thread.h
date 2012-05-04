#ifndef THREAD_H
#define THREAD_H

#include "pt.h"

typedef struct _thread_t {
    struct _thread_t *next;
    struct pt pt;
    PT_THREAD((*func)(struct _thread_t*));
} thread_t;

#define THREAD_RUN(thread) ((thread).func(&(thread)))
#define THREAD_RUN_ALL(threads) do { \
        for (thread_t *thread = (threads); thread; thread = thread->next) \
            THREAD_RUN(*thread); \
    } while (0);

#define THREAD(name) PT_THREAD(name ## _func(thread_t *self)); \
    thread_t name = { .func = name ## _func }; \
    PT_THREAD(name ## _func(thread_t *self))

#define THREAD_INIT(thread) PT_INIT(&(thread).pt)
#define THREAD_BEGIN() PT_BEGIN(&self->pt)
#define THREAD_END() PT_END(&self->pt)
#define THREAD_EXIT() PT_EXIT(&self->pt)
#define THREAD_RESTART() PT_RESTART(&self->pt)
#define THREAD_WAIT_UNTIL(condition) PT_WAIT_UNTIL(&self->pt, (condition))
#define THREAD_WAIT_WHILE(condition) PT_WAIT_WHILE(&self->pt, (condition))
#define THREAD_YIELD_UNTIL(condition) PT_YIELD_UNTIL(&self->pt, (condition))
#define THREAD_YIELD() PT_YIELD(&self->pt)

#endif
